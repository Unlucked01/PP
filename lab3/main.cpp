#include <mpi.h>
#include <iostream>
#include "car_race.h"
#include "matrix.h"
#include "matrix_mult.h"
#include "generator.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            std::cout << "Usage: " << argv[0] << " [car_race|matrix|matrix_v2]\n";
        }
        MPI_Finalize();
        return 1;
    }

    std::string mode(argv[1]);

    if (mode == "car_race") {
        if (size != CarRace::NUM_CARS + 1) {
            if (rank == 0) {
                std::cerr << "Car race requires " << CarRace::NUM_CARS + 1 
                         << " processes\n";
            }
            MPI_Finalize();
            return 1;
        }

        if (CarRace::isReferee(rank)) {
            runRefereeProcess();
        } else if (CarRace::isCarProcess(rank)) {
            runCarProcess(rank);
        }
    }
    else if (mode == "matrix") {
        if (size != 4) {
            if (rank == 0) {
                std::cerr << "Matrix multiplication variant 1 requires 4 processes\n";
            }
            MPI_Finalize();
            return 1;
        }
        if (rank == 0) {
            // Initialize matrices
            Matrix A(4, 5), B(5, 6), C(4, 6);
            Generator gen;

            // Fill matrices with random values
            for (int i = 0; i < A.getRows(); i++)
                for (int j = 0; j < A.getCols(); j++)
                    A.at(i, j) = gen.generateMatrixElement();

            for (int i = 0; i < B.getRows(); i++)
                for (int j = 0; j < B.getCols(); j++)
                    B.at(i, j) = gen.generateMatrixElement();

            std::cout << "Matrix A:\n";
            A.print();
            std::cout << "\nMatrix B:\n";
            B.print();

            double startTime = MPI_Wtime();
            variant1_rowDistribution(A, B, C);
            double endTime = MPI_Wtime();

            std::cout << "\nResult Matrix C:\n";
            C.print();
            std::cout << "Computation time: " << (endTime - startTime) 
                     << " seconds\n";
        }
        else {
            // Create named matrices for worker processes
            Matrix A(4, 5), B(5, 6), C(4, 6);
            variant1_rowDistribution(A, B, C);
        }
    }
    else if (mode == "matrix_v2") {
        if (size != 20) {
            if (rank == 0) {
                std::cerr << "Matrix multiplication variant 2 requires 20 processes\n";
            }
            MPI_Finalize();
            return 1;
        }
        
        if (rank == 0) {
            Matrix A(4, 5), B(5, 6), C(4, 6);
            Generator gen;

            // Fill matrices with random values
            for (int i = 0; i < A.getRows(); i++)
                for (int j = 0; j < A.getCols(); j++)
                    A.at(i, j) = gen.generateMatrixElement();

            for (int i = 0; i < B.getRows(); i++)
                for (int j = 0; j < B.getCols(); j++)
                    B.at(i, j) = gen.generateMatrixElement();

            std::cout << "Matrix A:\n";
            A.print();
            std::cout << "\nMatrix B:\n";
            B.print();

            double startTime = MPI_Wtime();
            variant2_elementDistribution(A, B, C);
            double endTime = MPI_Wtime();

            std::cout << "\nResult Matrix C (Variant 2):\n";
            C.print();
            std::cout << "Computation time: " << (endTime - startTime) 
                     << " seconds\n";
        }
        else {
            Matrix A(4, 5), B(5, 6), C(4, 6);
            variant2_elementDistribution(A, B, C);
        }
    }

    MPI_Finalize();
    return 0;
} 