#ifndef CAR_RACE_H
#define CAR_RACE_H

#include <mpi.h>
#include <vector>
#include <string>

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
    
    // Points allocation for positions
    static const int POINTS[5];

    static bool isReferee(int rank) {
        return rank == REFEREE_RANK;
    }

    static bool isCarProcess(int rank) {
        return rank > REFEREE_RANK && rank <= NUM_CARS;
    }

    static int getCarId(int rank) {
        return rank;
    }
};

void runRefereeProcess();
void runCarProcess(int rank);

#endif // CAR_RACE_H 