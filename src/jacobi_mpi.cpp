#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>

// --- CONFIGURACIÓN ---
const int N = 1000;          
const double MAX_ITER = 5000; // Bajamos un poco para que no esperes tanto en la prueba
const double TOL = 1e-4;       

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime(); 

    // 1. DESCOMPOSICIÓN
    int rows_base = N / size;
    int remainder = N % size;
    int local_rows = (rank < remainder) ? rows_base + 1 : rows_base;

    int size_alloc = (local_rows + 2) * N;
    std::vector<double> T_old(size_alloc, 0.0);
    std::vector<double> T_new(size_alloc, 0.0);

    // 2. INICIALIZACIÓN
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

    // 3. BUCLE PRINCIPAL
    while (iter < MAX_ITER) {
        MPI_Sendrecv(&T_old[1 * N], N, MPI_DOUBLE, top_neighbor, 0,
                     &T_old[0], N, MPI_DOUBLE, top_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&T_old[local_rows * N], N, MPI_DOUBLE, bottom_neighbor, 0,
                     &T_old[(local_rows + 1) * N], N, MPI_DOUBLE, bottom_neighbor, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        double local_diff = 0.0;

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

        // Convergencia
        if (iter % 100 == 0) {
            MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
            
            // --- ESTO FALTABA: IMPRIMIR PROGRESO ---
            if (rank == 0) {
                std::cout << "Iteracion " << iter << " - Error: " << global_diff << std::endl;
            }
            
            if (global_diff < TOL) break;
        }
        iter++;
    }

    double end_time = MPI_Wtime();

    if (rank == 0) {
        std::cout << "=== FINALIZADO ===" << std::endl;
        std::cout << "Procesos: " << size << " | Iteraciones: " << iter 
                  << " | Tiempo: " << (end_time - start_time) << " s" << std::endl;
    }

    // 4. RECOLECCIÓN
    std::vector<double> final_grid;
    std::vector<int> recvcounts;
    std::vector<int> displs;

    if (rank == 0) {
        final_grid.resize(N * N);
        recvcounts.resize(size);
        displs.resize(size);
        int current_displ = 0;
        for (int r = 0; r < size; r++) {
            int r_rows = (r < remainder) ? rows_base + 1 : rows_base;
            recvcounts[r] = r_rows * N;
            displs[r] = current_displ;
            current_displ += r_rows * N;
        }
    }

    MPI_Gatherv(&T_old[1 * N], local_rows * N, MPI_DOUBLE,
                final_grid.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::ofstream outfile("final_temp.txt");
        outfile << "Matriz " << N << "x" << N << " (Muestreo)" << std::endl;
        for (int i = 0; i < N; i += 20) { 
            for (int j = 0; j < N; j += 20) {
                outfile << final_grid[i * N + j] << " ";
            }
            outfile << "\n";
        }
        outfile.close();
        std::cout << "Archivo final_temp.txt generado." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
