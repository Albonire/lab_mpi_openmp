#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N 1000000
#endif

int main(int argc, char** argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        fprintf(stderr, "Error: MPI no soporta MPI_THREAD_FUNNELED\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0) {
        if (rank == 0)
            fprintf(stderr, "Error: N=%d no es divisible entre el numero de procesos (%d)\n", N, size);
        MPI_Finalize();
        return 1;
    }

    int chunk = N / size;
    long long *arr = NULL;

    if (rank == 0) {
        arr = (long long*) malloc((long long)N * sizeof(long long));
        if (arr == NULL) {
            fprintf(stderr, "Error reservando memoria para arr\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (long long i = 0; i < N; i++)
            arr[i] = i;
    }

    long long *local = (long long*) malloc((long long)chunk * sizeof(long long));
    if (local == NULL) {
        fprintf(stderr, "Error reservando memoria para local en rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_inicio = MPI_Wtime();

    MPI_Scatter(arr, chunk, MPI_LONG_LONG,
                local, chunk, MPI_LONG_LONG,
                0, MPI_COMM_WORLD);

    long long suma_local = 0;
    #pragma omp parallel for reduction(+:suma_local)
    for (int i = 0; i < chunk; i++)
        suma_local += local[i];

    long long suma_total = 0;
    MPI_Reduce(&suma_local, &suma_total, 1,
               MPI_LONG_LONG, MPI_SUM,
               0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double t_fin = MPI_Wtime();

    if (rank == 0) {
        long long suma_seq = 0;
        double ts = MPI_Wtime();
        for (long long i = 0; i < N; i++)
            suma_seq += arr[i];
        double te = MPI_Wtime();

        double tiempo_paralelo    = t_fin - t_inicio;
        double tiempo_secuencial  = te - ts;
        long long esperado        = (long long)N * (N - 1) / 2;

        printf("Configuracion: %d procesos MPI x %d hilos OpenMP\n", size, omp_get_max_threads());
        printf("Suma paralela   = %lld\n", suma_total);
        printf("Suma secuencial = %lld\n", suma_seq);
        printf("Esperado        = %lld\n", esperado);
        printf("Tiempo paralelo:    %.6f s\n", tiempo_paralelo);
        printf("Tiempo secuencial:  %.6f s\n", tiempo_secuencial);
        if (tiempo_paralelo > 0.0)
            printf("Speedup: %.2fx\n", tiempo_secuencial / tiempo_paralelo);
        else
            printf("Speedup: N/A (tiempo paralelo demasiado pequeno)\n");
    }

    free(arr);
    free(local);
    MPI_Finalize();
    return 0;
}