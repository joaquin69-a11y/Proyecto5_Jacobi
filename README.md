# Proyecto 5 – Simulación de Distribución de Temperatura 2D (MPI)

## 1. Resumen Ejecutivo
Este proyecto implementa una simulación numérica de la difusión de calor en una placa metálica bidimensional hasta alcanzar un estado estable, utilizando el Método de Jacobi. La solución fue paralelizada con OpenMPI empleando un modelo de memoria distribuida bajo la técnica de Descomposición del Dominio, lo que permite acelerar el cálculo distribuyendo la malla entre múltiples procesos.

El sistema gestiona la comunicación entre procesos mediante intercambio de fronteras (Halo Exchange) para garantizar la consistencia matemática durante la simulación.

---

## 2. Integrantes del Grupo
- [Tu Nombre]  
- [Integrante 2]  
- [Integrante 3]  
- [Integrante 4]

---

## 3. Estructura del Proyecto
La siguiente estructura organiza el proyecto de forma modular:

```text
Proyecto5_Jacobi/
├── README.md               # Informe y documentación
├── .gitignore              # Archivos ignorados
├── final_temp.txt          # Matriz final resultante
├── graficar.py             # Script de visualización (Heatmap)
├── jacobi_mpi              # Ejecutable principal (MPI)
└── src/
    ├── jacobi_mpi.cpp          # Implementación paralela
    ├── jacobi_secuencial.cpp   # Versión secuencial de referencia
    └── jacobi_mpi_setup.cpp    # Pruebas iniciales con MPI
```
---

# 4. Fundamento Teórico

## 4.1. El Problema Físico
Se simula una placa cuadrada de dimensión N × N con:

- Borde superior: 100°C  
- Bordes inferior, izquierdo y derecho: 0°C

Estas son condiciones de Dirichlet y generan un gradiente térmico natural.

---

## 4.2. Método de Jacobi
La actualización de la temperatura en cada punto se define como:

T(i,j)^(k+1) = 1/4 [ T(i+1,j)^k + T(i-1,j)^k + T(i,j+1)^k + T(i,j-1)^k ]

El proceso iterativo continúa hasta que el error máximo global sea menor a una tolerancia o se alcance un máximo de iteraciones.

---

# 5. Diseño e Implementación Paralela

## 5.1. Descomposición del Dominio
Se utiliza una división por filas (Row Decomposition):

- La matriz global de tamaño N se divide entre los procesos P.
- Cada proceso maneja N/P filas.
- Si existe residuo, se reparte entre los primeros procesos para balanceo de carga.

---

## 5.2. Comunicación de Fronteras (Halo Exchange)
Para calcular una fila local, se requieren datos del proceso vecino. Por eso se añaden:

- Halo superior: última fila real del proceso rank − 1  
- Halo inferior: primera fila real del proceso rank + 1  

Se utiliza MPI_Sendrecv para evitar interbloqueos y permitir recibir y enviar datos simultáneamente.

---

## 5.3. Sincronización Global
Cada proceso calcula su error local máximo.  
Luego se usa MPI_Allreduce con la operación MPI_MAX para obtener el error global y decidir si detener las iteraciones.

---

## 5.4. Recolección de Resultados
Al finalizar:

- El proceso raíz (rank 0) reconstruye la matriz completa con MPI_Gatherv.
- Se genera el archivo final_temp.txt.

---

# 6. Instalación y Ejecución

## 6.1. Requisitos
- Git  
- OpenMPI (mpic++, mpirun)  
- Python 3 + numpy + matplotlib (opcional)

---

## 6.2. Clonar el Repositorio
git clone https://github.com/TU_USUARIO/Proyecto5_Jacobi.git  
cd Proyecto5_Jacobi

---

## 6.3. Compilación
mpic++ src/jacobi_mpi.cpp -o jacobi_mpi

---

## 6.4. Ejecución
Ejemplo con 4 procesos:

mpirun -np 4 ./jacobi_mpi

---

## 6.5. Visualización
python3 graficar.py

Genera: mapa_calor.png o resultado_final.png

---

# 7. Resultados y Análisis

## 7.1. Parámetros del experimento
- Tamaño: 1000 × 1000  
- Iteraciones: 5000  
- Entorno: WSL sobre Windows  

---

## 7.2. Tabla de Métricas

Procesos | Tiempo (s) | Speedup | Eficiencia
---------|------------|---------|-----------
1        | 116.52     | 1.00    | 100%
2        | 67.89      | 1.72    | 86%
4        | 53.26      | 2.19    | 55%

---

## 7.3. Ejemplo de Salida (final_temp.txt)
Matriz 1000x1000 (Muestreo cada 20 filas)
100.00   100.00   100.00   ... (Fuente de calor)
  0.00    68.91    68.91   ... (Difusión alta)
  0.00    23.01    23.01   ... (Difusión media)
  0.00     0.00     0.00   ... (Zona fría)

---

## 7.4. Análisis Crítico
Aceleración real: el speedup de 1.72× con 2 procesos demuestra un paralelismo efectivo.

Eficiencia decreciente:
- Con más procesos, cada uno recibe menos filas (menos cómputo útil).
- El tiempo de comunicación (MPI_Sendrecv) aumenta proporcionalmente.
- WSL introduce latencias adicionales.

---

# 8. Conclusiones
- La implementación paralela converge correctamente, igual que la versión secuencial.  
- La gestión de halos y sincronización evita condiciones de carrera e interbloqueos.  
- Se obtiene una reducción mayor al 50% del tiempo total, demostrando la eficacia de MPI para acelerar simulaciones físicas incluso con overhead de comunicación.


