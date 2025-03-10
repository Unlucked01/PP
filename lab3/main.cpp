#include "matrix_mult.h"
#include "car_race.h"
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            std::cout << "Usage: " << argv[0] << " [car_race|matrix4|matrix20]\n";
        }
        MPI_Finalize();
        return 1;
    }

    std::string mode(argv[1]);

    if (mode == "car_race") {
        if (size != CarRace::NUM_CARS + 1) {
            if (rank == 0) {
                std::cerr << "Car race requires " << CarRace::NUM_CARS + 1 << " processes\n";
            }
            MPI_Finalize();
            return 1;
        }
        if (CarRace::isReferee(rank)) {
            runRefereeProcess();
        } else if (CarRace::isCarProcess(rank)) {
            runCarProcess(rank);
        }
    } else if (mode == "matrix4") {
        multiply_matrices_mpi_4(rank, size);
    } else if (mode == "matrix20") {
        multiply_matrices_mpi_20(rank, size);
    } else {
        if (rank == 0) {
            std::cout << "Invalid mode. Use: car_race, matrix4, or matrix20\n";
        }
    }

    MPI_Finalize();
    return 0;
} 