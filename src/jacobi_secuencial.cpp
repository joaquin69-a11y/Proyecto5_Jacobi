#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <algorithm>

// Parámetros de la simulación
const int N = 100;           // Tamaño de la malla (prueba con 100, luego 1000)
const double MAX_ITER = 5000;// Máximo de iteraciones
const double TOL = 1e-4;     // Tolerancia para detenerse

int main() {
    // 1. Crear malla (N x N)
    std::vector<double> T_old(N * N, 0.0); // Temperatura tiempo t
    std::vector<double> T_new(N * N, 0.0); // Temperatura tiempo t+1

    // 2. Inicializar Fronteras (Top=100, Bottom=0, Resto=0)
    for (int j = 0; j < N; j++) {
        T_old[0 * N + j] = 100.0;       // Fila superior (caliente)
        T_old[(N - 1) * N + j] = 0.0;   // Fila inferior (fría)
    }
    // Copiar condiciones iniciales a T_new
    T_new = T_old;

    std::cout << "Iniciando simulacion secuencial..." << std::endl;

    // 3. Bucle Principal (Jacobi)
    int iter = 0;
    double max_diff = 0.0;

    while (iter < MAX_ITER) {
        max_diff = 0.0;

        // Recorrer puntos INTERNOS (sin tocar bordes)
        for (int i = 1; i < N - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                // Ecuación de Jacobi: Promedio de los 4 vecinos
                double up    = T_old[(i - 1) * N + j];
                double down  = T_old[(i + 1) * N + j];
                double left  = T_old[i * N + (j - 1)];
                double right = T_old[i * N + (j + 1)];

                double val = 0.25 * (up + down + left + right);
                
                T_new[i * N + j] = val;

                // Calcular diferencia para convergencia
                double diff = std::abs(val - T_old[i * N + j]);
                if (diff > max_diff) max_diff = diff;
            }
        }

        // Actualizar mallas
        T_old = T_new; 
        
        iter++;
        // Chequeo de convergencia
        if (max_diff < TOL) {
            std::cout << "Convergencia alcanzada en iteracion " << iter << std::endl;
            break;
        }
    }

    // 4. Guardar resultado para validación
    std::ofstream file("resultado_secuencial.txt");
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            file << T_old[i*N + j] << " ";
        }
        file << "\n";
    }
    file.close();
    std::cout << "Archivo resultado_secuencial.txt generado." << std::endl;

    return 0;
}