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

