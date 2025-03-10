#ifndef CAR_RACE_H
#define CAR_RACE_H

#include <mpi.h>
#include <vector>
#include <string>
#include <random>
#include <chrono>

struct RaceResult {
    int carId;
    double stageTime;
    int position;
    int points;
};

class CarRace {
public:
    static const int NUM_CARS = 5;
    static const int NUM_STAGES = 3;
    static const int REFEREE_RANK = 0;
    static const int PROGRESS_TAG = 100;
    static const int RESULT_TAG = 200;
    static const int TRACK_LENGTH = 40;
    
    static constexpr int POINTS[5] = {10, 8, 6, 4, 2};

    static bool isReferee(int rank) {
        return rank == REFEREE_RANK;
    }

    static bool isCarProcess(int rank) {
        return rank > REFEREE_RANK && rank <= NUM_CARS;
    }

    static int getCarId(int rank) {
        return rank;
    }

    static int generateRaceDelay(int minMs, int maxMs) {
        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> dist(minMs, maxMs);
        return dist(rng);
    }
};

void runRefereeProcess();
void runCarProcess(int rank);

#endif // CAR_RACE_H 