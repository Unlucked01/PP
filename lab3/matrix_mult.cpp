#include <mpi.h>
#include <iostream>
#include "matrix.h"
#include "generator.h"

void variant1_rowDistribution(const Matrix& A, const Matrix& B, Matrix& C) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) {
            std::cerr << "Variant 1 requires exactly 4 processes\n";
        }
        MPI_Finalize();
        exit(1);
    }

    if (rank == 0) {
        auto Bdata = B.toArray();
        MPI_Bcast(Bdata.data(), Bdata.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

        auto Adata = A.toArray();
        std::vector<double> rowData(5);
        for (int i = 1; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                rowData[j] = A.at(i, j);
            }
            MPI_Send(rowData.data(), 5, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }

        std::vector<double> result(6, 0.0);
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 5; k++) {
                result[j] += A.at(0, k) * B.at(k, j);
            }
        }

        for (int i = 1; i < 4; i++) {
            std::vector<double> procResult(6);
            MPI_Recv(procResult.data(), 6, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < 6; j++) {
                C.at(i, j) = procResult[j];
            }
        }

        for (int j = 0; j < 6; j++) {
            C.at(0, j) = result[j];
        }
    }
    else if (rank < 4) {
        std::vector<double> Bdata(30);
        MPI_Bcast(Bdata.data(), 30, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        Matrix localB(5, 6);
        localB.fromArray(Bdata);

        std::vector<double> rowData(5);
        MPI_Recv(rowData.data(), 5, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<double> result(6, 0.0);
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 5; k++) {
                result[j] += rowData[k] * localB.at(k, j);
            }
        }

        MPI_Send(result.data(), 6, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
}

void variant2_elementDistribution(const Matrix& A, const Matrix& B, Matrix& C) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 20) {
        if (rank == 0) {
            std::cerr << "Variant 2 requires exactly 20 processes (4x6 result matrix)\n";
        }
        MPI_Finalize();
        exit(1);
    }

    // Process 0 is the coordinator
    if (rank == 0) {
        // Broadcast matrices A and B to all processes
        auto Adata = A.toArray();
        auto Bdata = B.toArray();
        
        MPI_Bcast(Adata.data(), Adata.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(Bdata.data(), Bdata.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Calculate one element (0,0) in the coordinator process
        double sum = 0.0;
        for (int k = 0; k < A.getCols(); k++) {
            sum += A.at(0, k) * B.at(k, 0);
        }
        C.at(0, 0) = sum;

        // Receive results from all worker processes
        for (int i = 1; i < size; i++) {
            double result;
            int row, col;
            
            MPI_Status status;
            MPI_Recv(&result, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&row, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&col, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            
            C.at(row, col) = result;
        }
    }
    else if (rank < 20) {
        // Worker processes
        // Receive broadcasted matrices
        std::vector<double> Adata(20); // 4x5 matrix
        std::vector<double> Bdata(30); // 5x6 matrix
        
        MPI_Bcast(Adata.data(), 20, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(Bdata.data(), 30, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        Matrix localA(4, 5), localB(5, 6);
        localA.fromArray(Adata);
        localB.fromArray(Bdata);

        // Calculate which element this process is responsible for
        // We need to distribute 24 elements (4x6 matrix) among 19 workers
        // Process 0 calculates (0,0), so we need to handle the remaining 23 elements
        int elementIndex = rank - 1;
        int row = elementIndex / 6;  // Integer division for row index
        int col = elementIndex % 6;  // Remainder for column index

        if (row < 4 && col < 6) {  // Check if this is a valid element
            // Calculate assigned matrix element
            double result = 0.0;
            for (int k = 0; k < localA.getCols(); k++) {
                result += localA.at(row, k) * localB.at(k, col);
            }

            // Send result and position back to coordinator
            MPI_Send(&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            MPI_Send(&row, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&col, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        }
    }
} 