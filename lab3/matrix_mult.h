#ifndef MATRIX_MULT_H
#define MATRIX_MULT_H

#include <vector>
#include <mpi.h>
#include <iostream>
#include <random>
#include <array>
#include <chrono>

template<size_t Rows, size_t Cols>
void print_matrix(const std::array<std::array<int, Cols>, Rows>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

void multiply_matrices_mpi_20(int rank, int size);
void multiply_matrices_mpi_4(int rank, int size);

#endif // MATRIX_MULT_H 