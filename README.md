# Proyecto 5: Simulación de Distribución de Temperatura 2D (MPI)

## 1. Resumen Ejecutivo
[cite_start]Este proyecto implementa una simulación numérica de la difusión de calor en una placa metálica bidimensional ($N \times N$) hasta alcanzar el estado estacionario[cite: 6]. [cite_start]La solución utiliza el **Método iterativo de Jacobi** [cite: 7] y ha sido paralelizada utilizando **OpenMPI** bajo un modelo de memoria distribuida. [cite_start]El objetivo principal es acelerar el cálculo mediante la descomposición del dominio espacial y la gestión eficiente de la comunicación entre procesos vecinos (Halo Exchange)[cite: 10, 11, 12].

## 2. Integrantes del Grupo
* [Tu Nombre]
* [Nombre Integrante 2]
* [Nombre Integrante 3]
* [Nombre Integrante 4]

---

## 3. Estructura del Proyecto
[cite_start]El código está organizado de manera modular siguiendo la estructura sugerida[cite: 37, 38]:

```text
Proyecto5_Jacobi/
├── README.md               # Informe Científico y documentación
├── .gitignore              # Archivos ignorados (binarios, temporales)
├── final_temp.txt          # Matriz de resultados (Salida del programa) 
├── graficar.py             # Script de Python para visualización (Mapa de Calor)
├── jacobi_mpi              # Ejecutable final compilado
└── src/
    ├── jacobi_mpi.cpp      # CÓDIGO FINAL: Implementación paralela optimizada
    ├── jacobi_secuencial.cpp # Versión secuencial para validación base [cite: 14]
    └── jacobi_mpi_setup.cpp  # Pruebas iniciales de entorno MPI

## 4. Fundamento Teórico

### 4.1. El Problema Físico
Se simula una placa cuadrada de dimensión $N \times N$. Se aplican condiciones de frontera de Dirichlet fijas:
* **Borde Superior:** $100^\circ C$ (Fuente de calor).
* **Bordes Inferior, Izquierdo y Derecho:** $0^\circ C$ (Sumideros).

### 4.2. Método de Jacobi
La temperatura $T$ en un punto $(i,j)$ para la iteración $k+1$ se calcula como el promedio de sus cuatro vecinos inmediatos (Arriba, Abajo, Izquierda, Derecha):

$$T_{i,j}^{k+1} = \frac{1}{4} (T_{i-1,j}^{k} + T_{i+1,j}^{k} + T_{i,j-1}^{k} + T_{i,j+1}^{k})$$

Este proceso iterativo continúa hasta que la diferencia máxima global entre iteraciones es menor a una tolerancia $\epsilon$ ($10^{-4}$).

---

## 5. Diseño e Implementación Paralela

### 5.1. Descomposición de Dominio
Se implementó una **división por filas** (Row Decomposition). La matriz global de $N$ filas se reparte entre $P$ procesos.
* Cada proceso calcula un bloque de filas locales ($N/P$).
* Se maneja el residuo ($N \% P$) asignando filas extra a los primeros rangos para asegurar un balanceo de carga adecuado.

### 5.2. Comunicación de Fronteras (Halo Exchange)
Dado que el cálculo de los bordes locales requiere datos que poseen los procesos vecinos, se implementaron **Filas Fantasma (Halos)**:
* **Halo Superior (Fila 0):** Almacena la última fila real del vecino $Rank-1$.
* **Halo Inferior (Fila N+1):** Almacena la primera fila real del vecino $Rank+1$.

**Estrategia MPI:** Se utilizó `MPI_Sendrecv` en cada iteración. Esta función permite enviar y recibir datos simultáneamente, evitando condiciones de carrera y *deadlocks* (interbloqueos) que podrían ocurrir con envíos bloqueantes simples.

### 5.3. Sincronización Global
Para verificar la convergencia, cada proceso calcula su error local máximo. Luego, se utiliza `MPI_Allreduce` con la operación `MPI_MAX` para que todos los procesos conozcan el error máximo global y decidan si detenerse.

### 5.4. Recolección de Resultados (I/O)
Al finalizar, el proceso raíz (Rank 0) reconstruye la matriz completa utilizando `MPI_Gatherv` (necesario debido a que los procesos pueden tener diferente cantidad de filas) y escribe el archivo `final_temp.txt` con un formato ordenado y un muestreo de datos para facilitar la lectura.

---

## 6. Instrucciones de Instalación y Ejecución

Estas instrucciones permiten descargar, compilar y ejecutar el proyecto en un entorno Linux/WSL.

### 6.1. Requisitos Previos
* Git
* Compilador MPI (`mpic++`)
* Python 3 con `matplotlib` y `numpy` (Opcional, para graficar)

### 6.2. Clonar el Repositorio
```bash
git clone [https://github.com/](https://github.com/)[TU_USUARIO]/Proyecto5_Jacobi.git
cd Proyecto5_Jacobi

### 6.3. Compilación
Para generar el ejecutable paralelo:
```bash
mpic++ src/jacobi_mpi.cpp -o jacobi_mpi

### 6.4. Ejecución
Para ejecutar la simulación con 4 procesos (ejemplo):
```bash
mpirun -np 4 ./jacobi_mpi
El programa mostrará el progreso cada 100 iteraciones y el tiempo total al finalizar.

### 6.5. Visualización de Resultados
Para generar el mapa de calor (gráfico):
```bash
python3 graficar.py
Esto generará la imagen resultado_final.png (o mapa_calor.png).

## 7. Resultados y Análisis de Rendimiento

**Configuración del Experimento:**
* **Tamaño de Malla ($N$):** $1000 \times 1000$ (1 millón de celdas).
* **Iteraciones:** 5000 pasos fijos.
* **Hardware:** Entorno WSL sobre Windows.

### 7.1. Tabla de Métricas Obtenidas

| Procesos ($P$) | Tiempo ($T_p$) | Speedup ($S = T_1/T_p$) | Eficiencia ($E = S/P$) |
| :---: | :---: | :---: | :---: |
| **1** (Secuencial) | **116.52 s** | 1.00 | 100% |
| **2** | **67.89 s** | 1.72 | 86% |
| **4** | **53.26 s** | 2.19 | 55% |

### 7.2. Ejemplo de Salida Real
Fragmento del archivo `final_temp.txt` generado, mostrando el gradiente térmico desde la fuente (arriba) hacia el sumidero (abajo):

```text
Matriz 1000x1000 (Muestreo cada 20 filas)
100.00   100.00   100.00   ... (Fuente)
  0.00    68.91    68.91   ... (Caliente)
  0.00    23.01    23.01   ... (Tibio)
  0.00     0.00     0.00   ... (Frío)

### 7.3. Análisis Crítico del Rendimiento
1.  **Speedup Positivo:** Se observa una mejora significativa al pasar de 1 a 2 procesos ($1.72x$). Esto valida que la estrategia de paralelización es correcta y efectiva para dividir la carga computacional.
2.  **Impacto de la Comunicación:** Al aumentar a 4 procesos, la eficiencia disminuye al 55%. Esto ocurre debido al **Overhead de Comunicación**:
    * En una malla de tamaño intermedio ($N=1000$), al dividir el trabajo entre 4, cada proceso realiza menos cálculos (250 filas cada uno), haciendo que el tiempo invertido en la transferencia de datos de los halos (`MPI_Sendrecv`) sea proporcionalmente mayor respecto al tiempo de cómputo útil.
    * La ejecución en un entorno virtualizado (WSL) añade una latencia adicional en la gestión de procesos que no ocurriría en un cluster físico dedicado.

---

## 8. Conclusión
El proyecto cumple exitosamente con los requisitos funcionales y académicos:
1.  [cite_start]**Convergencia Correcta:** La implementación paralela produce los mismos resultados físicos que la versión secuencial, cumpliendo con el criterio de funcionalidad[cite: 17].
2.  [cite_start]**Robustez:** El manejo de Halos y la sincronización evitan condiciones de carrera y *deadlocks* (interbloqueos), asegurando una ejecución estable[cite: 33].
3.  [cite_start]**Mejora de Tiempo:** Se logró reducir el tiempo de ejecución en más de un **50%** utilizando computación paralela, demostrando la eficacia de MPI para acelerar problemas de simulación física[cite: 10].
