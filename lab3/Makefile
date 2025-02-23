# Try to detect MPI compiler wrapper
ifneq ($(shell which mpicxx 2>/dev/null),)
    CXX = mpicxx
else ifneq ($(shell which mpic++ 2>/dev/null),)
    CXX = mpic++
else
    $(error MPI C++ compiler wrapper not found. Please install OpenMPI or MPICH)
endif

CXXFLAGS = -Wall -std=c++17 -O2

SRCS = main.cpp car_race.cpp matrix_mult.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = generator.h matrix.h car_race.h matrix_mult.h
TARGET = mpi_lab3

.PHONY: all clean run_car_race run_matrix run_matrix_v2 help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

run_car_race: $(TARGET)
	mpirun --host localhost --oversubscribe -np 6 ./$(TARGET) car_race

run_matrix: $(TARGET)
	mpirun -np 4 ./$(TARGET) matrix

run_matrix_v2: $(TARGET)
	mpirun --host localhost --oversubscribe -np 20 ./$(TARGET) matrix_v2

# Print help information
help:
	@echo "Available targets:"
	@echo "  all         - Build the program (default)"
	@echo "  clean       - Remove built files"
	@echo "  run_car_race - Run car race simulation (requires 6 processes)"
	@echo "  run_matrix  - Run matrix multiplication (requires 4 processes)"
	@echo "  run_matrix_v2 - Run matrix multiplication variant 2 (requires 20 processes)"
	@echo "  help        - Show this help message" 