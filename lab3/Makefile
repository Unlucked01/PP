CXX = mpic++
CXXFLAGS = -Wall -std=c++17
OBJECTS = matrix_mult.o car_race.o main.o

all: matrix_mult 1

matrix_mult: $(OBJECTS)
	$(CXX) $(OBJECTS) -o matrix_mult

1: 1.cpp
	$(CXX) $(CXXFLAGS) 1.cpp -o 1

matrix_mult.o: matrix_mult.cpp matrix_mult.h
	$(CXX) $(CXXFLAGS) -c matrix_mult.cpp

car_race.o: car_race.cpp car_race.h
	$(CXX) $(CXXFLAGS) -c car_race.cpp

main.o: main.cpp matrix_mult.h car_race.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o matrix_mult 1

run4: matrix_mult
	mpirun -np 4 --oversubscribe ./matrix_mult matrix4

run20: matrix_mult
	mpirun -np 20 --oversubscribe ./matrix_mult matrix20

run_race: matrix_mult
	mpirun -np 6 --oversubscribe ./matrix_mult car_race

run1: 1
	mpirun -np 2 --oversubscribe ./1

.PHONY: all clean run4 run20 run_race run1