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
// DEBUG is defined via compiler flag

int receive(int source, MPI_Comm comm);
void send(int destination, int value, MPI_Comm comm);
void print_array(const char* label, int* array, int size);


// Sort using dependency graph approach (2D Cartesian topology)
int *sort_dependency_graph(int input_array[SIZE], int rank, MPI_Comm comm) {
    int min, max;
    int coords[2];
    MPI_Cart_coords(comm, rank, DIMS_DG, coords);
    
    if (DEBUG) {
        MPI_Barrier(comm);
        if (rank == 0) cout << "\n--- Dependency Graph Sorting Debug Output ---" << endl;
        MPI_Barrier(comm);
        
        cout << "Process " << rank << " at coordinates (" << coords[0] << "," << coords[1] << ")" << endl;
        MPI_Barrier(comm);
    }
    
    if (coords[0] <= coords[1]) {
        int y_source, y_dest, x_source, x_dest;
        MPI_Cart_shift(comm, 0, 1, &y_source, &y_dest);
        MPI_Cart_shift(comm, 1, 1, &x_source, &x_dest);
        
        if (DEBUG) {
            cout << "Process " << rank << " neighbors: y_source=" << y_source 
                 << ", y_dest=" << y_dest << ", x_source=" << x_source 
                 << ", x_dest=" << x_dest << endl;
        }
        
        min = (coords[0] == coords[1]) ? INT_MAX : receive(x_source, comm);
        
        if (DEBUG) {
            if (coords[0] == coords[1]) {
                cout << "Process " << rank << " is on diagonal, setting min to INT_MAX" << endl;
            } else {
                cout << "Process " << rank << " received min=" << min << " from process " << x_source << endl;
            }
        }
        
        if (coords[0] == 0) {
            if (coords[1] == 0) {
                max = input_array[0];
                if (DEBUG) {
                    cout << "Process " << rank << " (0,0) setting max to input_array[0]=" << max << endl;
                }
                for (int i = 1; i < SIZE; ++i) {
                    send(i, input_array[i], comm);
                    if (DEBUG) {
                        cout << "Process " << rank << " sent input_array[" << i << "]=" 
                             << input_array[i] << " to process " << i << endl;
                    }
                }
            } else {
                max = receive(0, comm);
                if (DEBUG) {
                    cout << "Process " << rank << " received max=" << max << " from process 0" << endl;
                }
            }
        } else {
            max = receive(y_source, comm);
            if (DEBUG) {
                cout << "Process " << rank << " received max=" << max << " from process " << y_source << endl;
            }
        }
        
        if (min > max) {
            if (DEBUG) {
                cout << "Process " << rank << " swapping min=" << min << " and max=" << max << endl;
            }
            swap(min, max);
        }
        
        if (x_dest < 0) {
            send(0, min, comm);
            if (DEBUG) {
                cout << "Process " << rank << " sent min=" << min << " to process 0" << endl;
            }
        } else {
            send(x_dest, min, comm);
            if (DEBUG) {
                cout << "Process " << rank << " sent min=" << min << " to process " << x_dest << endl;
            }
        }
        
        if (coords[0] != coords[1]) {
            send(y_dest, max, comm);
            if (DEBUG) {
                cout << "Process " << rank << " sent max=" << max << " to process " << y_dest << endl;
            }
        }
        
        if (rank == 0) {
            int *result = new int[SIZE];
            int target = 0;
            for (int i = 0; i < SIZE; ++i) {
                if (i == 0)
                    target += SIZE - 1;
                else
                    target += SIZE;
                if (DEBUG) {
                    cout << "Process 0 waiting to receive result[" << i << "] from process " << target << endl;
                }
                int current_item = receive(target, comm);
                result[i] = current_item;
                if (DEBUG) {
                    cout << "Process 0 received result[" << i << "]=" << current_item << " from process " << target << endl;
                }
            }
            if (DEBUG) {
                cout << "Process 0 final sorted array: ";
                for (int i = 0; i < SIZE; i++) {
                    cout << result[i] << " ";
                }
                cout << endl;
            }
            return result;
        }
    }
    return nullptr;
}

// Sort using signal flow graph approach (1D Cartesian topology)
int *sort_signal_flow(int input_array[SIZE], int rank, MPI_Comm comm) {
    int min = INT_MAX;
    int source, dest;
    MPI_Cart_shift(comm, 0, 1, &source, &dest);
    
    if (DEBUG) {
        MPI_Barrier(comm);
        if (rank == 0) cout << "\n--- Signal Flow Graph Sorting Debug Output ---" << endl;
        MPI_Barrier(comm);
        
        cout << "Process " << rank << " neighbors: source=" << source << ", dest=" << dest << endl;
        if (rank == 0) {
            cout << "Process 0 input array: ";
            for (int i = 0; i < SIZE; i++) {
                cout << input_array[i] << " ";
            }
            cout << endl;
        }
        MPI_Barrier(comm);
    }
    
    for (int i = 0; i < SIZE; ++i) {
        int max = rank == 0 ? input_array[i] : receive(source, comm);
        if (DEBUG) {
            if (rank == 0) {
                cout << "Process " << rank << " using input_array[" << i << "]=" << max << endl;
            } else {
                cout << "Process " << rank << " received max=" << max << " from process " << source << endl;
            }
        }
        
        if (min > max) {
            if (DEBUG) {
                cout << "Process " << rank << " swapping min=" << min << " and max=" << max << endl;
            }
            swap(min, max);
            if (dest >= 0) {
                send(dest, max, comm);
                if (DEBUG) {
                    cout << "Process " << rank << " sent max=" << max << " to process " << dest << endl;
                }
            }
        } else if (dest >= 0) {
            send(dest, max, comm);
            if (DEBUG) {
                cout << "Process " << rank << " sent max=" << max << " to process " << dest << endl;
            }
        }
    }
    
    if (DEBUG) {
        cout << "Process " << rank << " final min value: " << min << endl;
        MPI_Barrier(comm);
    }
    
    int *result = rank == 0 ? new int[SIZE] : nullptr;

    if (rank == 0) {
        result[0] = min;
        if (DEBUG) {
            cout << "Process 0 setting result[0]=" << min << endl;
        }
        for (int i = 1; i < SIZE; i++) {
            if (DEBUG) {
                cout << "Process 0 waiting to receive result[" << i << "] from process " << i << endl;
            }
            result[i] = receive(i, comm);
            if (DEBUG) {
                cout << "Process 0 received result[" << i << "]=" << result[i] << " from process " << i << endl;
            }
        }
        if (DEBUG) {
            cout << "Process 0 final sorted array: ";
            for (int i = 0; i < SIZE; i++) {
                cout << result[i] << " ";
            }
            cout << endl;
        }
    } else {
        send(0, min, comm);
        if (DEBUG) {
            cout << "Process " << rank << " sent min=" << min << " to process 0" << endl;
        }
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

void send(int destination, int value, MPI_Comm comm) {
    MPI_Send(&value, 1, MPI_INT, destination, 0, comm);
}