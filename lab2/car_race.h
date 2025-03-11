#ifndef CAR_RACE_H
#define CAR_RACE_H

#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>


struct ProgressMsg {
    long mtype;      // Message type (car ID)
    int progress;    // Current position on track
};

struct ResultMsg {
    long mtype;      // Message type (car ID)
    double stageTime; // Time taken to complete the stage
};

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
    static const int REFEREE_ID = 0;
    static const int TRACK_LENGTH = 40;
    
    static constexpr int POINTS[5] = {10, 8, 6, 4, 2};

    static const key_t SEM_KEY = 0x1234;
    static const key_t PROGRESS_QUEUE_KEY = 0x2345;
    static const key_t RESULT_QUEUE_KEY = 0x3456;
    
    static const int SEM_START = 0;  // Used to signal race start
    static const int SEM_FINISH = 1; // Used to count finished cars

    static int generateRaceDelay(int minMs, int maxMs) {
        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> dist(minMs, maxMs);
        return dist(rng);
    }
};

void runRefereeProcess();
void runCarProcess(int carId);
void initializeIPC();
void cleanupIPC();
void clearMessageQueue(int qid);

#endif // CAR_RACE_H 