#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <stdexcept>
#include <iostream>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    int rows;
    int cols;

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }

    double& at(int i, int j) {
        if (i < 0 || i >= rows || j < 0 || j >= cols) {
            throw std::out_of_range("Matrix index out of bounds");
        }
        return data[i][j];
    }

    const double& at(int i, int j) const {
        if (i < 0 || i >= rows || j < 0 || j >= cols) {
            throw std::out_of_range("Matrix index out of bounds");
        }
        return data[i][j];
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }

    // Convert matrix to contiguous array for MPI operations
    std::vector<double> toArray() const {
        std::vector<double> arr(rows * cols);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                arr[i * cols + j] = data[i][j];
            }
        }
        return arr;
    }

    // Load matrix from contiguous array
    void fromArray(const std::vector<double>& arr) {
        if (arr.size() != rows * cols) {
            throw std::runtime_error("Array size mismatch");
        }
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                data[i][j] = arr[i * cols + j];
            }
        }
    }

    // Print matrix
    void print() const {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

#endif // MATRIX_H 