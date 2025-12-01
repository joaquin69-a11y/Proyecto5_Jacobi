#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

const int N = 100;             
const double MAX_ITER = 20000; 
const double TOL = 1e-4;       

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_base = N / size;
    int remainder = N % size;
    int local_rows = (rank < remainder) ? rows_base + 1 : rows_base;

    int total_rows_alloc = local_rows + 2;
    int size_alloc = total_rows_alloc * N;

    std::vector<double> T_old(size_alloc, 0.0);
    std::vector<double> T_new(size_alloc, 0.0);

    // Inicialización
    if (rank == 0) {
        for (int j = 0; j < N; j++) T_old[1 * N + j] = 100.0;
    }
    if (rank == size - 1) {
        for (int j = 0; j < N; j++) T_old[local_rows * N + j] = 0.0;
    }
    T_new = T_old;

    int top_neighbor = (rank == 0) ? MPI_PROC_NULL : rank - 1;
    int bottom_neighbor = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

    int iter = 0;
    double global_diff = 0.0;

    // --- INICIO DEL CÁLCULO ---
    while (iter < MAX_ITER) {
        
        // CORRECCIÓN: Usamos tag=0 para todo para evitar Deadlocks
        
        // 1. Intercambio con ARRIBA (Send arriba, Recv de arriba)
        MPI_Sendrecv(&T_old[1 * N], N, MPI_DOUBLE, top_neighbor, 0,
                     &T_old[0], N, MPI_DOUBLE, top_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. Intercambio con ABAJO (Send abajo, Recv de abajo)
        MPI_Sendrecv(&T_old[local_rows * N], N, MPI_DOUBLE, bottom_neighbor, 0,
                     &T_old[(local_rows + 1) * N], N, MPI_DOUBLE, bottom_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        double local_diff = 0.0;

        // Cálculo Jacobi
        for (int i = 1; i <= local_rows; i++) {
            if (rank == 0 && i == 1) continue; 
            if (rank == size - 1 && i == local_rows) continue;

            for (int j = 1; j < N - 1; j++) {
                int idx = i * N + j;
                double val = 0.25 * (T_old[(i-1)*N + j] + T_old[(i+1)*N + j] + 
                                     T_old[i*N + (j-1)] + T_old[i*N + (j+1)]);
                T_new[idx] = val;
                double diff = std::abs(val - T_old[idx]);
                if (diff > local_diff) local_diff = diff;
            }
        }

        T_old = T_new;

        MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        iter++;
        
        // NUEVO: Mostrar progreso para saber que no se colgó
        if (rank == 0 && iter % 1000 == 0) {
            std::cout << "Iteracion: " << iter << " - Error: " << global_diff << std::endl;
        }

        if (global_diff < TOL) break;
    }

    if (rank == 0) {
        std::cout << "=== FIN ===" << std::endl;
        std::cout << "Procesos: " << size << ". Total Iteraciones: " << iter << std::endl;
    }

    MPI_Finalize();
    return 0;
}
