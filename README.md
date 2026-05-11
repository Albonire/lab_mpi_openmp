# Lab MPI + OpenMP — Anderson Fabian Gonzalez

## Entorno de Trabajo

- **Sistema Operativo:** Fedora Linux
- **Compilador:** GCC (GNU Compiler Collection)
- **Implementacion MPI:** OpenMPI
- **Compilacion:** `mpicc` con flag `-fopenmp` para soporte hibrido
- **Hardware:** Configuracion de hilos logicos disponible en el equipo de desarrollo

---

## Ejercicio 1 — Hola Mundo MPI

**Descripcion breve:**  
Programa basico que introduce la estructura de un programa MPI. Cada proceso obtiene su identificador (`rank`) y el numero total de procesos (`size`), imprimiendo un saludo individual. El proceso con `rank == 0` actua como maestro e imprime un mensaje final de sincronizacion logica.

**Compilacion y ejecucion:**
```bash
mpicc -O2 mpi_01_hola.c -o mpi_01_hola
mpiexec -n 4 ./mpi_01_hola
mpiexec -n 2 ./mpi_01_hola
```

**Pantallazos:**

Ejecucion con 4 procesos:  
![Ejercicio 1 - 4 procesos](screenshots/ej1_4p.png)

Ejecucion con 2 procesos:  
![Ejercicio 1 - 2 procesos](screenshots/ej1_2p.png)

**Respuestas a preguntas de analisis:**

1. **Por que el orden de salida varia entre ejecuciones?**  
   Los procesos MPI se ejecutan de forma concurrente y asincrona. El planificador del sistema operativo es quien determina en que momento exacto cada proceso obtiene tiempo de CPU para imprimir. No existe una garantia de orden secuencial en las llamadas a `printf` provenientes de diferentes procesos.

2. **Que pasaria si ejecutas con -n 1? Tiene sentido paralelizar asi?**  
   Con `-n 1` solo existira el proceso `rank 0`. El programa funcionara correctamente pero no habra paralelismo real, ya que la ejecucion sera estrictamente secuencial. Paralelizar con un solo proceso carece de sentido practico mas alla de la depuracion.

3. **Para que sirve MPI_COMM_WORLD? Podria haber otros comunicadores?**  
   `MPI_COMM_WORLD` es el comunicador por defecto que agrupa a todos los procesos lanzados en la ejecucion. Si, es posible crear otros comunicadores mediante funciones como `MPI_Comm_split` para definir subgrupos de procesos que puedan comunicarse entre si de forma aislada del resto.

---

## Ejercicio 2 — OpenMP dentro de MPI

**Descripcion breve:**  
Introduccion al modelo de programacion hibrida. Se lanzan multiples procesos MPI, y dentro de cada uno se crea una region paralela de OpenMP que genera 4 hilos. Esto demuestra como combinar distribucion de memoria (MPI) con memoria compartida (OpenMP).

**Estructura del modelo hibrido:**

![Estructura del modelo híbrido](./screenshots/modelo-híbrido.svg)

**Compilacion y ejecucion:**
```bash
mpicc -O2 -fopenmp mpi_02_hibrido.c -o mpi_02_hibrido
mpiexec -n 2 ./mpi_02_hibrido
mpiexec -n 4 ./mpi_02_hibrido
```

**Pantallazos:**

Ejecucion con 2 procesos MPI y 4 hilos OMP:  
![Ejercicio 2 - 2 procesos MPI](screenshots/ej2_2mpi_4hilos.png)

Ejecucion con 4 procesos MPI y 4 hilos OMP:  
![Ejercicio 2 - 4 procesos MPI](screenshots/ej2_4mpi_4hilos.png)

**Respuestas a preguntas de analisis:**

1. **Con 2 procesos MPI y 4 hilos OMP, cuantas unidades de computo hay en total?**  
   Hay 8 unidades logicas de computo. Se calculan multiplicando la cantidad de procesos por la cantidad de hilos internos: 2 x 4 = 8.

2. **En que se diferencia ejecutar con -n 4 (4 MPI, 4 hilos) vs -n 1 (1 MPI, 16 hilos)?**  
   En la configuracion de 4 procesos MPI, la memoria esta distribuida en 4 espacios de direcciones separados; si necesitan compartir datos, deben usar comunicacion MPI (paso de mensajes). En la configuracion de 1 proceso y 16 hilos, toda la ejecucion ocurre en un solo espacio de memoria compartida, por lo que los hilos pueden leer y escribir las mismas variables directamente sin overhead de red, limitado por la capacidad de un solo nodo.

3. **Por que es importante MPI_Init_thread en lugar de MPI_Init cuando usamos OpenMP?**  
   Porque MPI necesita conocer el nivel de soporte de hilos que la aplicacion requerira. `MPI_Init_thread` permite solicitar un nivel especifico (en este caso `MPI_THREAD_FUNNELED`, que indica que solo el hilo principal realizara llamadas a la libreria MPI). Usar `MPI_Init` puede llevar a comportamientos indefinidos o errores en tiempo de ejecucion si multiples hilos intentan comunicarse via MPI simultaneamente.

---

## Ejercicio 3 — Suma Hibrida

**Descripcion breve:**  
El proceso raiz (`rank 0`) inicializa un vector grande de un millon de elementos. Se utiliza `MPI_Scatter` para distribuir porciones equitativas del vector a todos los procesos. Cada proceso suma su porcion local utilizando la paralelizacion de OpenMP con la clausula `reduction`. Finalmente, `MPI_Reduce` reune las sumas parciales en el proceso raiz para obtener la suma total.

**Flujo de datos del ejercicio:**

![Suma híbrida](./screenshots/suma.svg)

**Compilacion y ejecucion:**
```bash
mpicc -O2 -fopenmp mpi_03_suma_hibrida.c -o mpi_03
mpiexec -n 4 ./mpi_03
```

**Pantallazo:**

Resultado de la suma hibrida:  
![Ejercicio 3 - Resultado](screenshots/ej3_resultado.png)

**Respuestas a preguntas de analisis:**

1. **Que hace exactamente MPI_Scatter? Quien envia y quien recibe?**  
   `MPI_Scatter` toma un arreglo que reside en la memoria del proceso raiz (definido en los parametros, usualmente `rank 0`), lo divide en fragmentos iguales y envia un fragmento distinto a cada proceso del comunicador, incluyendose a si mismo. El proceso raiz es el emisor original, y todos los procesos (incluido el raiz) son receptores de su propio bloque.

2. **Por que usamos reduction(+:suma_local) y no una variable compartida directamente?**  
   Para evitar condiciones de carrera (race conditions). Si multiples hilos intentan escribir y leer la misma variable `suma_local` al mismo tiempo de forma descoordinada, se produciran perdidas de datos y el resultado sera incorrecto. La clausula `reduction` crea copias privadas de la variable para cada hilo y se encarga de combinarlas de forma segura al final de la region paralela.

3. **Que pasaria si olvidaras MPI_Reduce y solo imprimieras suma_local en rank==0?**  
   Solo se imprimiria la suma del bloque local que le correspondio al proceso 0 (los primeros 250,000 elementos). Las sumas parciales calculadas por los procesos 1, 2 y 3 se perderian al finalizar sus ejecuciones, dando un resultado drasticamente menor al esperado.

---

## Ejercicio 4 (Reto) — Speedup

**Descripcion breve:**  
A partir del codigo del ejercicio 3, se agrego medicion de tiempos usando `MPI_Wtime()` y una version puramente secuencial del calculo. El objetivo es comparar el tiempo de ejecucion secuencial frente a diferentes configuraciones de paralelismo (Solo MPI, Solo OpenMP, e Hibrido) para calcular el speedup y analizar la escalabilidad.

**Compilacion y ejecucion:**
```bash
mpicc -O2 -fopenmp -DN=8000000 mpi_04_speedup.c -o mpi_04_speedup
```

**Tabla de resultados:**

| Configuracion | Procesos MPI | Hilos OMP | Speedup Obtenido |
|---------------|:------------:|:---------:|:----------------:|
| Solo MPI      | 4            | 1         | [Completar] x    |
| Solo OMP      | 1            | 4         | [Completar] x    |
| MPI + OMP     | 2            | 2         | [Completar] x    |
| MPI + OMP     | 4            | 2         | [Completar] x    |

*(Nota: Reemplazar "[Completar]" con los valores obtenidos en tus ejecuciones)*

**Pantallazos:**

Configuracion Solo MPI:  
![Ejercicio 4 - Solo MPI](screenshots/ej4_solo_mpi.png)

Configuracion Solo OMP:  
![Ejercicio 4 - Solo OMP](screenshots/ej4_solo_omp.png)

Configuracion Hibrida 2x2:  
![Ejercicio 4 - MPI+OMP 2x2](screenshots/ej4_2x2.png)

Configuracion Hibrida 4x2:  
![Ejercicio 4 - MPI+OMP 4x2](screenshots/ej4_4x2.png)

**Analisis:**

1. **Coincide el speedup con lo que predice la Ley de Amdahl? (parte paralela approx 100%)**  
   Teoricamente, al ser un problema sumamente paralelizable, el speedup deberia acercarse al numero de unidades de computo. Sin embargo, en la practica el speedup es menor al ideal. La Ley de Amdahl predice el maximo teorico, pero no contempla los tiempos muertos por sincronizacion, el inicio de procesos, ni las latencias de red, por lo que rara vez se alcanza una escalabilidad lineal perfecta.

2. **Por que mas procesos/hilos no siempre significan mayor speedup?**  
   Existen varios factores limitantes. Primero, el hardware tiene un numero fisico de núcleos; lanzar mas hilos o procesos de los disponibles causa "oversubscription", generando cambios de contexto costosos. Segundo, a medida que se aumenta el paralelismo, el overhead de comunicacion (en MPI) y de sincronizacion de hilos crece, hasta el punto donde cuesta mas coordinarse que hacer el calculo en si.

3. **Que overhead introduce MPI que no existe en OpenMP puro?**  
   MPI introduce el costo de la comunicacion entre procesos (paso de mensajes por la pila de red o sockets locales), la copia de datos entre espacios de memoria separados, y la sincronizacion global implicita en operaciones colectivas como `Scatter` y `Reduce`. OpenMP puro opera dentro de un mismo espacio de memoria compartida, evitando las copias de datos y el costo de transmision por red.

---

## Conclusiones

- La programacion hibrida MPI + OpenMP permite aprovechar las ventajas de ambos paradigmas: la distribucion a traves de multiples nodos con MPI y la explotacion de núcleos locales con memoria compartida mediante OpenMP.
- La sincronizacion y el manejo correcto de las variables (usando `reduction` en OpenMP y comunicadores en MPI) son estrictamente necesarios para evitar resultados erroneos causados por condiciones de carrera.
- La medicion de rendimiento demuestra que el paralelismo no es una solucion magica; el overhead de comunicacion y las limitaciones del hardware imponen un limite a la ganancia de velocidad (speedup), haciendo fundamental el analisis empirico para determinar la configuracion optima de procesos e hilos.