CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

all: car_race

car_race: main.cpp car_race.cpp car_race.h
	$(CXX) $(CXXFLAGS) main.cpp car_race.cpp -o car_race

run: car_race
	./car_race

clean:
	rm -f car_race 