#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>

using namespace std;

#define SIZE 5
#define DIMS_DG 2
#define DIMS_SFG 1

int receive(int source, MPI_Comm comm);
void print_array(const char* label, int* array, int size);


int *sort_dependency_graph(int input_array[SIZE], int rank, MPI_Comm comm) {
    int min, max;
    int coords[2];
    MPI_Cart_coords(comm, rank, DIMS_DG, coords);
    
    if (coords[0] <= coords[1]) {
        int y_source, y_dest, x_source, x_dest;
        MPI_Cart_shift(comm, 0, 1, &y_source, &y_dest);
        MPI_Cart_shift(comm, 1, 1, &x_source, &x_dest);
        
        min = (coords[0] == coords[1]) ? INT_MAX : receive(x_source, comm);
        
        if (coords[0] == 0) {
            if (coords[1] == 0) {
                max = input_array[0];
                for (int i = 1; i < SIZE; ++i)
                    MPI_Send(&input_array[i], 1, MPI_INT, i, 0, comm);
            } else {
                max = receive(0, comm);
            }
        } else {
            max = receive(y_source, comm);
        }
        
        if (min > max) swap(min, max);
        
        if (x_dest < 0) {
            MPI_Send(&min, 1, MPI_INT, 0, 0, comm);
        } else {
            MPI_Send(&min, 1, MPI_INT, x_dest, 0, comm);
        }
        
        if (coords[0] != coords[1])
            MPI_Send(&max, 1, MPI_INT, y_dest, 0, comm);
        
        if (rank == 0) {
            int *result = new int[SIZE];
            int target = 0;
            for (int i = 0; i < SIZE; ++i) {
                if (i == 0)
                    target += SIZE - 1;
                else
                    target += SIZE;
                int current_item = receive(target, comm);
                result[i] = current_item;
            }
            return result;
        }
    }
    return nullptr;
}

int *sort_signal_flow(int input_array[SIZE], int rank, MPI_Comm comm) {
    int min = INT_MAX;
    int source, dest;
    MPI_Cart_shift(comm, 0, 1, &source, &dest);
    
    for (int i = 0; i < SIZE; ++i) {
        int max = rank == 0 ? input_array[i] : receive(source, comm);
        if (min > max) {
            swap(min, max);
        }
        if (dest >= 0) {
            MPI_Send(&max, 1, MPI_INT, dest, 0, comm);
        }
    }
    
    int *result = rank == 0 ? new int[SIZE] : nullptr;

    if (rank == 0) {
        result[0] = min;
        for (int i = 1; i < SIZE; i++) {
            result[i] = receive(i, comm);
        }
    } else {
        MPI_Send(&min, 1, MPI_INT, 0, 0, comm);
    }
    return result;
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Comm comm;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    bool use_dependency_graph = (size > SIZE);
    int input_array[SIZE] = {2, 5, 3, 1, 4};
    int *result = nullptr;
    auto start_time = chrono::high_resolution_clock::now();
    
    if (rank == 0) {
        print_array("Input array", input_array, SIZE);
    }
    
    if (use_dependency_graph) {
        int dims[DIMS_DG] = {0, 0};
        int periods[DIMS_DG] = {0, 0};
        
        MPI_Dims_create(size, DIMS_DG, dims);
        MPI_Cart_create(MPI_COMM_WORLD, DIMS_DG, dims, periods, 1, &comm);
        
        if (rank == 0) {
            cout << "Using dependency graph approach with " << dims[0] << "×" << dims[1] << " topology" << endl;
        }
        
        result = sort_dependency_graph(input_array, rank, comm);
    } else {
        int dims[DIMS_SFG] = {0};
        int periods[DIMS_SFG] = {0};
        
        MPI_Dims_create(size, DIMS_SFG, dims);
        MPI_Cart_create(MPI_COMM_WORLD, DIMS_SFG, dims, periods, 1, &comm);
        
        if (rank == 0) {
            cout << "Using signal flow graph approach with " << dims[0] << " processes" << endl;
        }
        
        result = sort_signal_flow(input_array, rank, comm);
    }
    
    if (rank == 0) {
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
        
        print_array("Sorted array", result, SIZE);
        cout << "Sorting completed in " << duration.count() << " microseconds" << endl;
        
        delete[] result;
    }
    
    MPI_Finalize();
    return 0;
}

void print_array(const char* label, int* array, int size) {
    cout << label << ": ";
    for (int i = 0; i < size; ++i) {
        cout << setw(3) << array[i] << " ";
    }
    cout << endl;
}

int receive(int source, MPI_Comm comm) {
    int value;
    MPI_Status status;
    MPI_Recv(&value, 1, MPI_INT, source, MPI_ANY_TAG, comm, &status);
    return value;
}