#ifndef MATRIX_MULT_H
#define MATRIX_MULT_H

#include "matrix.h"

// Function to perform matrix multiplication using row distribution (Variant 1)
void variant1_rowDistribution(const Matrix& A, const Matrix& B, Matrix& C);

// Function to perform matrix multiplication using element distribution (Variant 2)
void variant2_elementDistribution(const Matrix& A, const Matrix& B, Matrix& C);

#endif // MATRIX_MULT_H 