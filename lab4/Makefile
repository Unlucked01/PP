CXX = mpic++
CXXFLAGS = -Wall -std=c++17 -O2
DEBUG_FLAGS = -g -Wall -std=c++17 -DDEBUG=1
BUILD_DIR = build

$(shell mkdir -p $(BUILD_DIR))

all: parallel_sort matrix_vector_mult parallel_sort_debug matrix_vector_mult_debug

parallel_sort: $(BUILD_DIR)/parallel_sort.o
	$(CXX) $< -o $(BUILD_DIR)/$@

matrix_vector_mult: $(BUILD_DIR)/matrix_vector_mult.o
	$(CXX) $< -o $(BUILD_DIR)/$@

parallel_sort_debug: $(BUILD_DIR)/parallel_sort_debug.o
	$(CXX) $< -o $(BUILD_DIR)/$@

matrix_vector_mult_debug: $(BUILD_DIR)/matrix_vector_mult_debug.o
	$(CXX) $< -o $(BUILD_DIR)/$@

$(BUILD_DIR)/parallel_sort.o: 1.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/matrix_vector_mult.o: 2.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/parallel_sort_debug.o: 1_debug.cpp
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD_DIR)/matrix_vector_mult_debug.o: 2_debug.cpp
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run_cartesian: parallel_sort
	mpirun -np 25 --oversubscribe $(BUILD_DIR)/parallel_sort

run_graph: parallel_sort
	mpirun -np 5 --oversubscribe $(BUILD_DIR)/parallel_sort

run_matrix_ring: matrix_vector_mult
	mpirun -np 5 --oversubscribe $(BUILD_DIR)/matrix_vector_mult 1

run_matrix_line: matrix_vector_mult
	mpirun -np 5 --oversubscribe $(BUILD_DIR)/matrix_vector_mult 0

run_cartesian_debug: parallel_sort_debug
	mpirun -np 25 --oversubscribe $(BUILD_DIR)/parallel_sort_debug

run_graph_debug: parallel_sort_debug
	mpirun -np 4 --oversubscribe $(BUILD_DIR)/parallel_sort_debug

run_matrix_ring_debug: matrix_vector_mult_debug
	mpirun -np 5 --oversubscribe $(BUILD_DIR)/matrix_vector_mult_debug 1

run_matrix_line_debug: matrix_vector_mult_debug
	mpirun -np 5 --oversubscribe $(BUILD_DIR)/matrix_vector_mult_debug 0

.PHONY: all clean run_cartesian run_graph run_matrix_ring run_matrix_line run_cartesian_debug run_graph_debug run_matrix_ring_debug run_matrix_line_debug