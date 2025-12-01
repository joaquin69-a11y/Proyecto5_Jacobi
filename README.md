\# Proyecto 5: Simulación de Distribución de Temperatura 2D (MPI)



\## 1. Resumen

Este proyecto simula la difusión de calor en una placa metálica bidimensional hasta alcanzar un estado estable. El objetivo principal es acelerar el cálculo utilizando paralelismo de memoria distribuida con OpenMPI, dividiendo el dominio de la placa entre múltiples procesos computacionales.



\## 2. Fundamento Teórico

\### Método de Jacobi

La simulación utiliza el Método de Jacobi, un algoritmo iterativo para resolver ecuaciones diferenciales parciales. La temperatura de un punto $T(i,j)$ en el tiempo $t+1$ se calcula como el promedio de sus cuatro vecinos inmediatos en el tiempo $t$:



$$T\_{i,j}^{k+1} = \\frac{1}{4} (T\_{i-1,j}^{k} + T\_{i+1,j}^{k} + T\_{i,j-1}^{k} + T\_{i,j+1}^{k})$$



Esto implica que para calcular el nuevo estado de la malla, necesitamos conocer el estado completo de la iteración anterior.



\### Estrategia de Paralelización (Próxima etapa)

Se utilizará una \*\*Descomposición de Dominio por Filas\*\*. La matriz $N \\times N$ se dividirá horizontalmente. Cada proceso será responsable de actualizar una franja de filas. Para calcular los bordes de sus franjas, los procesos necesitarán intercambiar "filas fantasma" (halos) con sus vecinos superior e inferior.



\## 3. Estado del Proyecto (Semana 1)

\- \[x] Implementación secuencial verificada (`src/jacobi\_secuencial.cpp`).

\- \[x] Entorno MPI configurado y probado (`src/jacobi\_mpi\_setup.cpp`).

\- \[x] Repositorio inicializado.

## 4. Diseño e Implementación (Semana 2)

### 4.1 Descomposición del Dominio
Para paralelizar el problema, se utilizó una descomposición espacial 1D por filas.
* La matriz de tamaño $N \times N$ se divide entre $P$ procesos.
* Cada proceso es responsable de calcular un bloque de aproximadamente $N/P$ filas.
* **Manejo de residuos:** Si $N$ no es divisible exactamente por $P$, las filas sobrantes se asignan a los primeros rangos para balancear la carga.

### 4.2 Comunicación de Fronteras (Halo Exchange)
Dado que el cálculo de Jacobi en la fila $i$ requiere datos de las filas $i-1$ y $i+1$, los procesos en los bordes de su bloque local necesitan datos que residen en la memoria de otros procesos vecinos.

* **Estrategia:** Se añadieron dos filas "fantasma" (halos) a la memoria local de cada proceso:
    * `Fila 0`: Almacena la última fila del vecino superior.
    * `Fila Local+1`: Almacena la primera fila del vecino inferior.
* **Implementación MPI:** Se utilizó `MPI_Sendrecv` en cada iteración para realizar el intercambio de manera bidireccional y evitar *deadlocks* (interbloqueos).
    * El rango $R$ envía su primera fila real a $R-1$ y recibe en su halo superior.
    * El rango $R$ envía su última fila real a $R+1$ y recibe en su halo inferior.
## 5. Resultados y Análisis de Rendimiento (Semana 3)
Se realizaron pruebas incrementando el número de procesos con una malla de tamaño $N=1000$ durante 5000 iteraciones.

### Tabla de Resultados
| Procesos (P) | Tiempo (s) | Speedup ($S = T_1/T_p$) | Eficiencia ($E = S/P$) |
|--------------|------------|-------------------------|------------------------|
| 1            | 116.52     | 1.00                    | 1.00 (100%)            |
| 2            | 67.89      | 1.72                    | 0.86 (86%)             |
| 4            | 53.26      | 2.19                    | 0.55 (55%)             |

### Análisis Crítico
Se observa una mejora significativa al pasar de 1 a 2 procesos (Speedup de 1.72), lo que indica una buena paralelaización.
Sin embargo, al pasar a 4 procesos, la eficiencia cae al 55%. Esto ocurre debido al **Overhead de Comunicación**:
1. El tamaño del problema ($N=1000$) es relativamente pequeño para 4 procesos, por lo que el tiempo gastado en `MPI_Sendrecv` (latencia) empieza a competir con el tiempo de cálculo.
2. Al ejecutar en un entorno virtualizado (WSL), la gestión de múltiples procesos añade una sobrecarga adicional.

### Conclusión Final
El proyecto cumple con los objetivos: la simulación converge correctamente (validado en la Semana 2) y se logra una reducción del tiempo de ejecución superior al 50% al utilizar computación paralela con MPI.
