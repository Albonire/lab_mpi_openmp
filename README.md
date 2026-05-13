# LAB-01-MPI-OPENMP-HYBRID | Anderson Fabian Gonzalez

> **Asignatura:** Fundamentos de Programación Concurrente y Distribuida  
> **Docente:** Prf. Alejandro Jaimes  
> **Fecha:** 13/05/2025  
> **Repositorio:** [lab_mpi_openmp](https://github.com/Albonire/lab_mpi_openmp)

---

## Equipo

|   | Colaborador              | GitHub                                          |
|---|--------------------------|--------------------------------------------------|
| 👤 | Anderson Fabian Gonzalez | [@Albonire](https://github.com/Albonire)        |

**Repositorio:** [lab_mpi_openmp](https://github.com/Albonire/lab_mpi_openmp)  
**Rama principal:** `main`

## Configuración del repositorio

### Clonar

```bash
git clone https://github.com/Albonire/lab_mpi_openmp.git
cd lab_mpi_openmp
```

### Convención de commits

```
lab01: agrega ejercicio 1 hola mundo MPI
lab01: completa ejercicio 3 suma híbrida
lab01: agrega pantallazos ejercicio 2
fix:   corrige suma en mpi_03
```

## Estructura de Repo

```md
lab_mpi_openmp/
│
├── README.md
├── .gitignore
├── mpi_01_hola.c
├── mpi_02_hibrido.c
├── mpi_03_suma_hibrida.c
├── mpi_04_speedup.c
│
└── screenshots/
    ├── ej1_2p.png
    ├── ej1_4p.png
    ├── ej2_2mpi_4hilos.png
    ├── ej2_4mpi_4hilos.png
    ├── ej3_resultado.png
    ├── ej4_solo_mpi.png
    ├── ej4_solo_omp.png
    ├── ej4_2x2.png
    ├── ej4_4x2.png
    ├── modelo-híbrido.svg
    └── suma.svg
```

---

## Ejercicio 1 — Hola Mundo MPI

**Descripción:** Cada proceso MPI imprime su rank y el total de procesos. El proceso maestro (rank 0) imprime un mensaje adicional al final.

**Compilación y ejecución:**

```bash
mpicc mpi_01_hola.c -o mpi_01_hola
mpiexec -n 4 ./mpi_01_hola
mpiexec -n 2 ./mpi_01_hola
```

**Pantallazo — 4 procesos:**

![Ejercicio 1 con 4 procesos](screenshots/ej1_4p.png)

**Pantallazo — 2 procesos:**

![Ejercicio 1 con 2 procesos](screenshots/ej1_2p.png)

**Respuestas a las preguntas de análisis:**

1. **¿Por qué el orden de salida varía entre ejecuciones?**  
   Los procesos MPI se ejecutan de forma concurrente y asíncrona. El planificador del sistema operativo determina en qué momento cada proceso obtiene tiempo de CPU para imprimir. No existe garantía de orden secuencial en las llamadas a `printf` provenientes de diferentes procesos.

2. **¿Qué pasaría si ejecutas con `-n 1`?**  
   Con `-n 1` solo existirá el proceso `rank 0`. El programa funcionará correctamente pero no habrá paralelismo real, ya que la ejecución será estrictamente secuencial. Paralelizar con un solo proceso carece de sentido práctico más allá de la depuración.

3. **¿Para qué sirve `MPI_COMM_WORLD`?**  
   `MPI_COMM_WORLD` es el comunicador por defecto que agrupa a todos los procesos lanzados en la ejecución. Es posible crear otros comunicadores mediante funciones como `MPI_Comm_split` para definir subgrupos de procesos que se comuniquen entre sí de forma aislada.

---

## Ejercicio 2 — OpenMP dentro de MPI

**Descripción:** Dentro de cada proceso MPI se lanza una región paralela OpenMP con 4 hilos. Cada hilo imprime su ID junto con el rank del proceso que lo contiene. Al final, el maestro calcula el total de unidades de cómputo activas.

**Estructura del modelo híbrido:**

![Estructura del modelo híbrido](screenshots/modelo-híbrido.svg)

**Compilación y ejecución:**

```bash
mpicc -fopenmp mpi_02_hibrido.c -o mpi_02_hibrido
mpiexec -n 2 ./mpi_02_hibrido
mpiexec -n 4 ./mpi_02_hibrido
```

**Pantallazo — 2 procesos MPI × 4 hilos:**

![Ejercicio 2 con 2 procesos](screenshots/ej2_2mpi_4hilos.png)

**Pantallazo — 4 procesos MPI × 4 hilos:**

![Ejercicio 2 con 4 procesos](screenshots/ej2_4mpi_4hilos.png)

**Respuestas a las preguntas de análisis:**

1. **Con 2 procesos MPI y 4 hilos OMP, ¿cuántas unidades de cómputo hay?**  
   Hay 8 unidades lógicas de cómputo: 2 × 4 = 8.

2. **¿Diferencia entre `-n 4` (4 MPI, 4 hilos) vs `-n 1` (1 MPI, 16 hilos)?**  
   Con 4 procesos MPI, la memoria está distribuida en 4 espacios de direcciones separados; si necesitan compartir datos, deben usar paso de mensajes. Con 1 proceso y 16 hilos, toda la ejecución ocurre en un solo espacio de memoria compartida, evitando el overhead de comunicación por red, pero limitado a un solo nodo.

3. **¿Por qué `MPI_Init_thread` en lugar de `MPI_Init`?**  
   Porque MPI necesita conocer el nivel de soporte de hilos requerido. `MPI_Init_thread` permite solicitar `MPI_THREAD_FUNNELED`, indicando que solo el hilo principal realizará llamadas MPI. Usar `MPI_Init` puede llevar a comportamientos indefinidos si múltiples hilos intentan comunicarse vía MPI simultáneamente.

---

## Ejercicio 3 — Suma Híbrida de Vector

**Descripción:** El proceso maestro inicializa un vector de 1,000,000 de elementos. Se utiliza `MPI_Scatter` para distribuir porciones equitativas a todos los procesos. Cada proceso suma su porción local con OpenMP usando `reduction`. Finalmente, `MPI_Reduce` reúne las sumas parciales en el proceso raíz.

**Flujo de datos:**

![Suma híbrida](screenshots/suma.svg)

**Compilación y ejecución:**

```bash
mpicc -fopenmp mpi_03_suma_hibrida.c -o mpi_03
mpiexec -n 4 ./mpi_03
```

**Pantallazo — resultado:**

![Ejercicio 3 resultado](screenshots/ej3_resultado.png)

**Verificación:**

```
Suma total = 499999500000
Esperado   = 499999500000  ✓
```

**Respuestas a las preguntas de análisis:**

1. **¿Qué hace exactamente `MPI_Scatter`?**  
   Toma un arreglo en el proceso raíz, lo divide en fragmentos iguales y envía un fragmento distinto a cada proceso del comunicador (incluyéndose a sí mismo). El proceso raíz es el emisor y todos los procesos son receptores de su propio bloque.

2. **¿Por qué `reduction(+:suma_local)` y no una variable compartida?**  
   Para evitar condiciones de carrera. Si múltiples hilos escriben y leen la misma variable simultáneamente, se producen pérdidas de datos. La cláusula `reduction` crea copias privadas para cada hilo y las combina de forma segura al final de la región paralela.

3. **¿Qué pasaría si olvidaras `MPI_Reduce` e imprimieras `suma_local` en rank 0?**  
   Solo se imprimiría la suma del bloque local del proceso 0 (los primeros 250,000 elementos). Las sumas parciales de los demás procesos se perderían, dando un resultado drásticamente menor al esperado.

---

## Ejercicio 4 (Reto) — Speedup Híbrido

**Descripción:** Se añade medición de tiempos con `MPI_Wtime()` al Ejercicio 3 para comparar el rendimiento secuencial frente a diferentes configuraciones de paralelismo y calcular el speedup.

**Compilación:**

```bash
mpicc -fopenmp -DN=8000000 mpi_04_speedup.c -o mpi_04_speedup
```

**Ejecuciones:**

```bash
OMP_NUM_THREADS=1 mpiexec -n 4 ./mpi_04_speedup   # Solo MPI
OMP_NUM_THREADS=4 mpiexec -n 1 ./mpi_04_speedup   # Solo OMP
OMP_NUM_THREADS=2 mpiexec -n 2 ./mpi_04_speedup   # Híbrido 2x2
OMP_NUM_THREADS=2 mpiexec -n 4 ./mpi_04_speedup   # Híbrido 4x2
```

**Tabla de resultados:**

| Configuración | Procesos MPI | Hilos OMP | Tiempo paralelo (s) | Tiempo secuencial (s) | Speedup |
|---------------|:------------:|:---------:|:-------------------:|:---------------------:|:-------:|
| Solo MPI      | 4            | 1         | 0.047853            | 0.005583              | 0.12×   |
| Solo OMP      | 1            | 4         | 0.045457            | 0.006333              | 0.14×   |
| MPI + OMP     | 2            | 2         | 0.049716            | 0.006862              | 0.14×   |
| MPI + OMP     | 4            | 2         | 0.090506            | 0.009596              | 0.11×   |

**Pantallazos:**

![Speedup solo MPI](screenshots/ej4_solo_mpi.png)
![Speedup solo OMP](screenshots/ej4_solo_omp.png)
![Speedup híbrido 2x2](screenshots/ej4_2x2.png)
![Speedup híbrido 4x2](screenshots/ej4_4x2.png)

**Respuestas a las preguntas de análisis:**

1. **¿Coincide con la Ley de Amdahl?**  
   En este caso los speedups obtenidos son menores a 1 (slowdown), lo cual indica que el overhead de inicialización de procesos/hilos y comunicación supera el beneficio del paralelismo para este tamaño de problema. La Ley de Amdahl predice el máximo teórico asumiendo overhead cero; en la práctica, con N=8,000,000 y operaciones simples de suma, el costo de `MPI_Scatter`/`MPI_Reduce` y la creación de hilos domina sobre el cómputo real.

2. **¿Por qué más procesos/hilos no siempre dan mayor speedup?**  
   El hardware tiene un número finito de núcleos físicos; lanzar más hilos o procesos causa oversubscription y cambios de contexto costosos. Además, a medida que aumenta el paralelismo, el overhead de comunicación (MPI) y sincronización crece hasta superar el beneficio del cómputo paralelo, como se evidencia en estos resultados.

3. **¿Qué overhead introduce MPI que no existe en OpenMP puro?**  
   MPI introduce el costo de comunicación entre procesos (paso de mensajes por sockets locales o red), la copia de datos entre espacios de memoria separados, y la sincronización global implícita en operaciones colectivas como `Scatter` y `Reduce`. OpenMP opera dentro de un mismo espacio de memoria compartida, evitando copias y transmisión.

---

## Conclusiones

1. La programación híbrida MPI + OpenMP permite combinar distribución entre nodos (MPI) con explotación de núcleos locales mediante memoria compartida (OpenMP), ofreciendo flexibilidad para diferentes arquitecturas de hardware.

2. El overhead de comunicación y sincronización no es despreciable: para problemas con operaciones simples (como sumas) y tamaños moderados, el costo de `MPI_Scatter`/`MPI_Reduce` y la creación de hilos puede superar el beneficio del paralelismo, resultando en slowdown en lugar de speedup.

3. La correcta sincronización de variables (usando `reduction` en OpenMP y operaciones colectivas en MPI) es estrictamente necesaria para evitar condiciones de carrera y resultados erróneos.

4. El análisis empírico del rendimiento es fundamental: la Ley de Amdahl proporciona un límite teórico, pero factores como la latencia de comunicación, el tamaño del problema y la granularidad del cómputo determinan si el paralelismo es realmente beneficioso en la práctica.

5. Para obtener speedup real en un esquema híbrido, se requiere un volumen de cómputo por proceso/hilo suficientemente grande que justifique el overhead de coordinación, lo cual implica escalar el tamaño del problema (N) o aumentar la complejidad computacional por elemento.
