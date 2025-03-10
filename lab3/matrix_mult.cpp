#include "matrix_mult.h"

void initialize_matrices(
    std::array<std::array<int, 5>, 4>& A, 
    std::array<std::array<int, 6>, 5>& B,
    std::array<std::array<int, 5>, 6>& BT) {
    std::mt19937 gen(1234);
    std::uniform_int_distribution<> dis(1, 10);
    
        for (auto& row : A) std::generate(row.begin(), row.end(), [&]{ return dis(gen); });
        for (auto& row : B) std::generate(row.begin(), row.end(), [&]{ return dis(gen); });

        std::cout << "Matrix A:\n";
        print_matrix(A);
        std::cout << "\nMatrix B:\n";
        print_matrix(B);

        for (int i = 0; i < B.size(); i++) 
            for (int j = 0; j < B[0].size(); j++) 
                BT[j][i] = B[i][j];
            
        std::cout << "\nMatrix B Transposed:\n";
        print_matrix(BT);
    std::cout << "--------------------------------" << std::endl;
}


void multiply_matrices_mpi_20(int rank, int size) {
    const int ROWS_A = 4, COLS_A = 5, ROWS_B = 5, COLS_B = 6;
    const int GROUP_SIZE = 5;

    std::array<std::array<int, COLS_A>, ROWS_A> A;
    std::array<std::array<int, COLS_B>, ROWS_B> B;
    std::array<std::array<int, ROWS_B>, COLS_B> BT;
    std::array<std::array<int, COLS_B>, ROWS_A> result;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    MPI_Comm leader_comm, member_comm;
    MPI_Comm_split(MPI_COMM_WORLD, rank % GROUP_SIZE == 0, rank, &leader_comm);
    MPI_Comm_split(MPI_COMM_WORLD, rank / GROUP_SIZE, rank, &member_comm);

    if (rank == 0) {
        initialize_matrices(A, B, BT);
    }

    int my_a, my_b;
    MPI_Scatter(A.data(), 1, MPI_INT, &my_a, 1, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < COLS_B; i++) {
        std::array<int, ROWS_B> b_row;
        MPI_Bcast(BT[i].data(), ROWS_B, MPI_INT, 0, leader_comm);
        b_row = BT[i];
        MPI_Scatter(b_row.data(), 1, MPI_INT, &my_b, 1, MPI_INT, 0, member_comm);
        
        int local_result = my_a * my_b;
        
        int group_result;
        MPI_Reduce(&local_result, &group_result, 1, MPI_INT, MPI_SUM, 0, member_comm);
        
        if (rank % GROUP_SIZE == 0) {
            int row_idx = rank / GROUP_SIZE;
            result[row_idx][i] = group_result;

            MPI_Gather(result[row_idx].data(), COLS_B, MPI_INT, result.data(), COLS_B, MPI_INT, 0, leader_comm);
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    
    if (rank == 0) {
        std::cout << "Result Matrix C:\n";
        print_matrix(result);
        std::cout << "20-processor computation time: " << duration << " microseconds" << std::endl;
    }

    MPI_Comm_free(&leader_comm);
    MPI_Comm_free(&member_comm);
}

void multiply_matrices_mpi_4(int rank, int size) {
    const int ROWS_A = 4, COLS_A = 5, ROWS_B = 5, COLS_B = 6;
   
    std::array<std::array<int, COLS_A>, ROWS_A> A;
    std::array<std::array<int, COLS_B>, ROWS_B> B;
    std::array<std::array<int, ROWS_B>, COLS_B> BT;
    std::array<std::array<int, COLS_B>, ROWS_A> result;

    auto start_time = std::chrono::high_resolution_clock::now();
    
    if (rank == 0) {
        initialize_matrices(A, B, BT);
    }

    std::array<int, COLS_A> my_a_row;
    MPI_Scatter(A.data(), COLS_A, MPI_INT, my_a_row.data(), COLS_A, MPI_INT, 0, MPI_COMM_WORLD);
    std::array<int, COLS_B> my_result_row;
    
    for (int j = 0; j < COLS_B; j++) {
        std::array<int, ROWS_B> bt_col;
        MPI_Bcast(BT[j].data(), ROWS_B, MPI_INT, 0, MPI_COMM_WORLD);
        bt_col = BT[j];
        
        int dot_product = 0;
        for (int k = 0; k < COLS_A; k++) {
            dot_product += my_a_row[k] * bt_col[k];
        }
        my_result_row[j] = dot_product;
    }
    
    MPI_Gather(my_result_row.data(), COLS_B, MPI_INT, result.data(), COLS_B, MPI_INT, 0, MPI_COMM_WORLD);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    
    if (rank == 0) {
        std::cout << "Result Matrix C:\n";
        print_matrix(result);
        std::cout << "4-processor computation time: " << duration << " microseconds" << std::endl;
    }
}