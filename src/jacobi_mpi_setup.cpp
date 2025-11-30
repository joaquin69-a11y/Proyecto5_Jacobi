#include <mpi.h>
#include <iostream>
#include <vector>

const int N = 100; // Tamaño de prueba

int main(int argc, char** argv) {
    // 1. Inicializar entorno MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 2. Lógica solo para el Proceso Raíz (Rank 0)
    if (rank == 0) {
        std::cout << "Soy el Proceso Raiz (0). El total de procesos es: " << size << std::endl;
        std::cout << "Inicializando malla maestra de " << N << "x" << N << "..." << std::endl;

        // Reservar memoria para toda la malla
        std::vector<double> Matrix(N * N, 0.0);

        // Inicializar bordes (solo para probar que accedemos a memoria)
        for (int j = 0; j < N; j++) {
            Matrix[0 * N + j] = 100.0; // Borde superior
        }

        std::cout << "Malla inicializada correctamente en memoria del Rank 0." << std::endl;
        std::cout << "Listo para la Fase 2 (Distribucion) la proxima semana." << std::endl;
    } 
    else {
        // Los otros procesos solo confirman asistencia
        // (Aún no reciben datos en la semana 1)
        // std::cout << "Soy el proceso trabajador " << rank << ", esperando datos..." << std::endl;
    }

    MPI_Finalize();
    return 0;
}