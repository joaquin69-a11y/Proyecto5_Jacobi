# Proyecto 5: Simulación de Distribución de Temperatura 2D (MPI)

## 1. Resumen Ejecutivo
Este proyecto aborda la simulación de la difusión de calor en una placa metálica bidimensional hasta alcanzar un estado estable, empleando el Método de Jacobi 2D ($N \times N$) . El objetivo principal fue acelerar cada iteración del cálculo distribuyendo la malla 2D entre múltiples procesos (MPI) , utilizando un modelo de Memoria Distribuida mediante la Descomposición del Dominio por filas. El desafío central es la Comunicación de Fronteras (Halo Exchange), donde cada proceso necesita intercambiar las filas/columnas limítrofes con sus vecinos en cada paso de tiempo antes de poder realizar el siguiente cálculo. Este informe detalla la estrategia de paralelización, la implementación del protocolo anti-interbloqueo, y presenta un análisis crítico del rendimiento (speedup y eficiencia).

## 2. Integrantes del Grupo
* Carlos Alberto Canahuiri Huarancca
* Alexander Tantahuillca Navarro
* Joaquin Luis Ibarra Cruz
* Cristian De la cruz Nateros

---

## 3. Estructura del Proyecto
El código está organizado de manera modular para facilitar la navegación:

```text
Proyecto5_Jacobi/
├── README.md               # Informe Científico y documentación
├── .gitignore              # Archivos ignorados (binarios, temporales)
├── final_temp.txt          # Matriz de resultados (Salida del programa)
├── graficar.py             # Script de Python para visualización (Mapa de Calor)
├── jacobi_mpi              # Ejecutable final compilado
└── src/
    ├── jacobi_mpi.cpp      # CÓDIGO FINAL: Implementación paralela optimizada
    ├── jacobi_secuencial.cpp # Versión secuencial para validación base
    └── jacobi_mpi_setup.cpp  # Pruebas iniciales de entorno MPI
````

-----

## 4\. Objetivos del proyecto

### 4.1. Objetivo General

Implementar una simulación paralela del método de Jacobi 2D utilizando memoria distribuida con MPI, con el fin de acelerar el cálculo de difusión de temperatura en una placa metálica y analizar el impacto del paralelismo en el rendimiento computacional.

### 4.2. Objetivos Específicos
Diseñar y construir una versión secuencial del método de Jacobi como referencia base.

  * Implementar la versión paralela mediante Descomposición del Dominio por filas.
  * Implementar el intercambio de fronteras (Halo Exchange) utilizando MPI_Sendrecv para evitar interbloqueos.
  * Garantizar que la solución paralela converja al mismo estado estable que la versión secuencial.
  * Evaluar el rendimiento mediante métricas de Speedup, Eficiencia y Tiempo total de ejecución.
  * Validar los resultados mediante visualización gráfica y análisis numérico.
  * Comparar el costo computacional entre la versión secuencial y paralela para diferentes números de procesos.

-----

## 5\. Alcance y Limitaciones

### 5.1. Alcance

  * Implementación del método iterativo de Jacobi para una malla 2D de tamaño configurable.
  * Simulación de condiciones de frontera fijas (Dirichlet).
  * Paralelización mediante MPI usando memoria distribuida.
  * División del dominio por filas con manejo explícito de halos.
  * Sincronización de error global mediante MPI_Allreduce.
  * Reconstrucción de resultados en el proceso raíz mediante MPI_Gatherv.
  * Generación de visualización en forma de mapa de calor utilizando Python.
  * Análisis de rendimiento siguiendo métricas clásicas de computación paralela.

### 5.2. Limitaciones

  * La comunicación entre procesos utiliza un esquema bloqueante (MPI_Sendrecv), por lo que no se explota la superposición de comunicación y cómputo.
  * No se implementa decomposición bidimensional (solo por filas), lo que puede limitar la escalabilidad para valores grandes de P.
  * La simulación está limitada a modelos 2D; no incluye versión 3D.
  * El rendimiento puede verse afectado por la ejecución en entornos virtualizados como WSL.
  * No se incorpora balanceo dinámico de carga; la división es estática.

-----

## 6\. Fundamento Teórico

### 6.1. El Problema Físico

Se simula una placa cuadrada de dimensión $N \times N$. Se aplican condiciones de frontera de Dirichlet fijas:

  * **Borde Superior:** $100^\circ C$ (Fuente de calor).
  * **Bordes Inferior, Izquierdo y Derecho:** $0^\circ C$ (Sumideros).

### 6.2. Método de Jacobi

La temperatura $T$ en un punto $(i,j)$ para la iteración $k+1$ se calcula como el promedio de sus cuatro vecinos inmediatos (Arriba, Abajo, Izquierda, Derecha). La ecuación de actualización es:

$$T_{i,j}^{k+1} = \frac{1}{4} (T_{i-1,j}^{k} + T_{i+1,j}^{k} + T_{i,j-1}^{k} + T_{i,j+1}^{k})$$

Este proceso iterativo continúa hasta que la diferencia máxima global entre iteraciones es menor a una tolerancia $\epsilon$ ($10^{-4}$).
Se necesitan dos mallas (una actual y una nueva) para implementar Jacobi, ya que los valores de $T^k$ deben mantenerse constantes durante la actualización de toda la malla para $T^{k+1}$13.

### 6.3. Descomposición de Dominio y Halos
La malla se divide horizontalmente (por filas) para distribuir el trabajo entre los procesos MPI.

El Halo (o Frontera) es la fila exterior de cada sub-matriz que pertenece al proceso vecino. Cuando la malla se divide por filas, un proceso necesita la última fila de su vecino superior y la primera fila de su vecino inferior.

La comunicación del "halo" es un requisito clave para el paralelismo
-----

## 7\. Diseño e Implementación Paralela

### 7.1. Descomposición de Dominio

Se implementó una **división por filas** (Row Decomposition). La matriz global de $N$ filas se reparte entre $P$ procesos.

  * Cada proceso calcula un bloque de filas locales ($N/P$).
  * Se maneja el residuo ($N \% P$) asignando filas extra a los primeros rangos para asegurar un balanceo de carga adecuado.

### 7.2. Comunicación de Fronteras (Halo Exchange)

Dado que el cálculo de los bordes locales requiere datos que poseen los procesos vecinos, se implementaron **Filas Fantasma (Halos)**:

  * **Halo Superior (Fila 0):** Almacena la última fila real del vecino $Rank-1$.
  * **Halo Inferior (Fila N+1):** Almacena la primera fila real del vecino $Rank+1$.

**Estrategia MPI:** Se utilizó `MPI_Sendrecv` en cada iteración. Esta función permite enviar y recibir datos simultáneamente, evitando condiciones de carrera y *deadlocks* (interbloqueos) que podrían ocurrir con envíos bloqueantes simples.

### 7.3. Sincronización Global

Para verificar la convergencia, cada proceso calcula su error local máximo. Luego, se utiliza `MPI_Allreduce` con la operación `MPI_MAX` para que todos los procesos conozcan el error máximo global y decidan si detenerse.

### 7.4. Recolección de Resultados (I/O)

Al finalizar, el proceso raíz (Rank 0) reconstruye la matriz completa utilizando `MPI_Gatherv` (necesario debido a que los procesos pueden tener diferente cantidad de filas) y escribe el archivo `final_temp.txt` con un formato ordenado y un muestreo de datos para facilitar la lectura.

### 7.5. Estructura del Código
| Archivo | Contenido Clave |
| :---: | :---: |
| main.c |Contiene la lógica MPI (inicialización, distribución, y recolección).|
| Jacobi.h | Clases/Funciones para la simulación: initialize_malla(), update_sequential(), y update_parallel_iter(). |

-----

## 8\. Instrucciones de Instalación y Ejecución

Estas instrucciones permiten descargar, compilar y ejecutar el proyecto en un entorno Linux/WSL utilizando la API de OpenMPI.

### 8.1. Requisitos Previos

  * Git
  * Compilador MPI (`mpic++`)
  * Python 3 con `matplotlib` y `numpy` (Opcional, para graficar)

### 8.2. Clonar el Repositorio

```bash
git clone [https://github.com/](https://github.com/)[TU_USUARIO]/Proyecto5_Jacobi.git
cd Proyecto5_Jacobi
```

### 8.3. Compilación

Para generar el ejecutable paralelo:

```bash
mpic++ src/jacobi_mpi.cpp -o jacobi_mpi
```

### 8.4. Ejecución

Para ejecutar la simulación con 4 procesos (ejemplo):

```bash
mpirun -np 4 ./jacobi_mpi
```

*El programa mostrará el progreso cada 100 iteraciones y el tiempo total al finalizar.*

### 8.5. Visualización de Resultados

Para generar el mapa de calor (gráfico):

```bash
python3 graficar.py
```

*Esto generará la imagen `resultado_final.png` (o `mapa_calor.png`).*

-----

## 9\. Resultados y Análisis de Rendimiento

**Configuración del Experimento:**

  * **Tamaño de Malla ($N$):** $1000 \times 1000$ (1 millón de celdas).
  * **Iteraciones:** 5000 pasos fijos.
  * **Hardware:** Entorno WSL sobre Windows.

### 9.1. Tabla de Métricas Obtenidas

| Procesos ($P$) | Tiempo ($T_p$) | Speedup ($S = T_1/T_p$) | Eficiencia ($E = S/P$) |
| :---: | :---: | :---: | :---: |
| **1** (Secuencial) | **116.77 s** | 1.00 | 100% |
| **2** | **67.60 s** | 1.73 | 86% |
| **4** | **52.88 s** | 2.21 | 55% |







