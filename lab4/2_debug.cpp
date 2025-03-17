#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>
#include <cstring>

#define MATRIX_SIZE 5
// DEBUG is defined via compiler flag

void ring_multiplication(int* local_matrix, int* local_vector, int* result, int rank, MPI_Comm comm);
void pipeline_multiplication(int* local_matrix, int* local_vector, int* vector, int* result, int rank, MPI_Comm comm);
void print_matrix(const char* label, const int* matrix, int rows, int cols);
void print_vector(const char* label, const int* vector, int size);

int main(int argc, char **argv) {
    int rank, size;
    MPI_Comm comm;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int matrix[MATRIX_SIZE * MATRIX_SIZE];
    int vector[MATRIX_SIZE];
    int result[MATRIX_SIZE] = {0};
    
    if (rank == 0) {
        std::mt19937 gen(1234);
        std::uniform_int_distribution<> dis(1, 10);
        
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            matrix[i] = dis(gen);
        }
        for (int i = 0; i < MATRIX_SIZE; i++) {
            vector[i] = dis(gen);
        }

        print_matrix("Input Matrix", matrix, MATRIX_SIZE, MATRIX_SIZE);
        print_vector("Input Vector", vector, MATRIX_SIZE);
    }
    
    int local_matrix[MATRIX_SIZE];
    int local_vector[1];
    
    bool use_ring_topology = false;
    if (argc > 1) {
        use_ring_topology = (atoi(argv[1]) != 0);
    }
    
    int dims[1] = {MATRIX_SIZE};
    int periods[1] = {use_ring_topology ? 1 : 0};  // Periodic for ring, non-periodic for pipeline
    
    MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 1, &comm);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    MPI_Scatter(matrix, MATRIX_SIZE, MPI_INT, local_matrix, MATRIX_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(vector, 1, MPI_INT, local_vector, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (use_ring_topology) {
        if (rank == 0) {
            std::cout << "Using ring topology for matrix-vector multiplication" << std::endl;
        }
        ring_multiplication(local_matrix, local_vector, result, rank, comm);
    } else {
        if (rank == 0) {
            std::cout << "Using pipeline topology for matrix-vector multiplication" << std::endl;
        }
        pipeline_multiplication(local_matrix, local_vector, vector, result, rank, comm);
    }
    
    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        print_vector("Result Vector", result, MATRIX_SIZE);
        std::cout << "Computation completed in " << duration.count() << " microseconds" << std::endl;
    }
    
    MPI_Comm_free(&comm);
    MPI_Finalize();
    return 0;
}

void ring_multiplication(int* local_matrix, int* local_vector, int* result, int rank, MPI_Comm comm) {
    int source, dest;
    int vector_elem = local_vector[0];
    int local_result = 0;
    
    MPI_Cart_shift(comm, 0, 1, &source, &dest);
    
    if (DEBUG) {
        // Synchronize to make debug output cleaner
        MPI_Barrier(comm);
        if (rank == 0) std::cout << "\n--- Ring Topology Debug Output ---" << std::endl;
        MPI_Barrier(comm);
        
        // Print topology information
        int coords[1];
        MPI_Cart_coords(comm, rank, 1, coords);
        std::cout << "Process " << rank << " at position " << coords[0] 
                  << " has neighbors: source=" << source << ", dest=" << dest << std::endl;
        std::cout << "Process " << rank << " initial vector element: " << vector_elem << std::endl;
        
        // Print local matrix row
        std::cout << "Process " << rank << " matrix row: ";
        for (int i = 0; i < MATRIX_SIZE; i++) {
            std::cout << local_matrix[i] << " ";
        }
        std::cout << std::endl;
        
        MPI_Barrier(comm);
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        int matrix_idx = (rank + MATRIX_SIZE - i) % MATRIX_SIZE;
        int partial_result = local_matrix[matrix_idx] * vector_elem;
        local_result += partial_result;
        
        if (DEBUG) {
            std::cout << "Process " << rank << " iteration " << i 
                      << ": matrix[" << matrix_idx << "]=" << local_matrix[matrix_idx]
                      << " * vector_elem=" << vector_elem 
                      << " = " << partial_result 
                      << " (running sum: " << local_result << ")" << std::endl;
        }
        
        int next_vector_elem;
        MPI_Sendrecv(&vector_elem, 1, MPI_INT, dest, 0,
                    &next_vector_elem, 1, MPI_INT, source, 0,
                    comm, MPI_STATUS_IGNORE);
        
        if (DEBUG) {
            std::cout << "Process " << rank << " sent " << vector_elem 
                      << " to process " << dest << " and received " 
                      << next_vector_elem << " from process " << source << std::endl;
        }
        
        vector_elem = next_vector_elem;
    }

    if (DEBUG) {
        std::cout << "Process " << rank << " final result: " << local_result << std::endl;
        MPI_Barrier(comm);
    }
    
    MPI_Gather(&local_result, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void pipeline_multiplication(int* local_matrix, int* local_vector, int* vector, int* result, int rank, MPI_Comm comm) {
    int source, dest;
    int local_result = 0;
    
    MPI_Cart_shift(comm, 0, 1, &source, &dest);
    
    if (DEBUG) {
        // Synchronize to make debug output cleaner
        MPI_Barrier(comm);
        if (rank == 0) std::cout << "\n--- Pipeline Topology Debug Output ---" << std::endl;
        MPI_Barrier(comm);
        
        // Print topology information
        int coords[1];
        MPI_Cart_coords(comm, rank, 1, coords);
        std::cout << "Process " << rank << " at position " << coords[0] 
                  << " has neighbors: source=" << source << ", dest=" << dest << std::endl;
        
        // Print local matrix row
        std::cout << "Process " << rank << " matrix row: ";
        for (int i = 0; i < MATRIX_SIZE; i++) {
            std::cout << local_matrix[i] << " ";
        }
        std::cout << std::endl;
        
        // Print local vector element
        std::cout << "Process " << rank << " local vector element: " << local_vector[0] << std::endl;
        
        if (rank == 0) {
            std::cout << "Process 0 full vector: ";
            for (int i = 0; i < MATRIX_SIZE; i++) {
                std::cout << vector[i] << " ";
            }
            std::cout << std::endl;
        }
        
        MPI_Barrier(comm);
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        int vector_elem;
        
        if (rank == 0) {
            vector_elem = vector[i];
            
            if (DEBUG) {
                std::cout << "Process 0 using vector[" << i << "] = " << vector_elem << std::endl;
            }
            
            if (dest >= 0) {
                MPI_Send(&vector_elem, 1, MPI_INT, dest, 0, comm);
                if (DEBUG) {
                    std::cout << "Process 0 sent " << vector_elem << " to process " << dest << std::endl;
                }
            }
        } else {
            MPI_Status status;
            MPI_Recv(&vector_elem, 1, MPI_INT, source, MPI_ANY_TAG, comm, &status);
            
            if (DEBUG) {
                std::cout << "Process " << rank << " received " << vector_elem 
                          << " from process " << source << std::endl;
            }
            
            if (dest >= 0) {
                MPI_Send(&vector_elem, 1, MPI_INT, dest, 0, comm);
                if (DEBUG) {
                    std::cout << "Process " << rank << " forwarded " << vector_elem 
                              << " to process " << dest << std::endl;
                }
            }
        }
        
        int partial_result = local_matrix[i] * vector_elem;
        local_result += partial_result;
        
        if (DEBUG) {
            std::cout << "Process " << rank << " iteration " << i 
                      << ": matrix[" << i << "]=" << local_matrix[i]
                      << " * vector_elem=" << vector_elem 
                      << " = " << partial_result 
                      << " (running sum: " << local_result << ")" << std::endl;
        }
    }
    
    if (DEBUG) {
        std::cout << "Process " << rank << " final result: " << local_result << std::endl;
        MPI_Barrier(comm);
    }
    
    MPI_Gather(&local_result, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void print_matrix(const char* label, const int* matrix, int rows, int cols) {
    std::cout << label << ":" << std::endl;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << std::setw(4) << matrix[i * cols + j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void print_vector(const char* label, const int* vector, int size) {
    std::cout << label << ": ";
    for (int i = 0; i < size; i++) {
        std::cout << std::setw(4) << vector[i];
    }
    std::cout << std::endl;
}