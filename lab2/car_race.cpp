#include "car_race.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <errno.h>

int semid = -1;
int progressQueueId = -1;
int resultQueueId = -1;

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void displayTrack(const std::vector<int>& positions) {
    for (int car = 1; car <= CarRace::NUM_CARS; car++) {
        std::cout << "Car " << car << " [";
        int pos = positions[car];
        for (int i = 0; i < CarRace::TRACK_LENGTH; i++) {
            if (i == pos) {
                std::cout << "🏎️ ";
            } else if (i == CarRace::TRACK_LENGTH - 1) {
                std::cout << "🏁";
            } else {
                std::cout << "-";
            }
        }
        std::cout << "]\n";
    }
    std::cout << std::flush;
}

void initializeIPC() {
    semid = semget(CarRace::SEM_KEY, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }

    semctl(semid, CarRace::SEM_START, SETVAL, 0);
    semctl(semid, CarRace::SEM_FINISH, SETVAL, 0);

    progressQueueId = msgget(CarRace::PROGRESS_QUEUE_KEY, IPC_CREAT | 0666);
    resultQueueId = msgget(CarRace::RESULT_QUEUE_KEY, IPC_CREAT | 0666);
    
    msgctl(progressQueueId, IPC_RMID, NULL);
    progressQueueId = msgget(CarRace::PROGRESS_QUEUE_KEY, IPC_CREAT | 0666);
    
    msgctl(resultQueueId, IPC_RMID, NULL);
    resultQueueId = msgget(CarRace::RESULT_QUEUE_KEY, IPC_CREAT | 0666);
}

void cleanupIPC() {
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);
    }
    if (progressQueueId != -1) {
        msgctl(progressQueueId, IPC_RMID, NULL);
    }
    if (resultQueueId != -1) {
        msgctl(resultQueueId, IPC_RMID, NULL);
    }
}

void runCarProcess(int carId) {
    struct sembuf ops;
    
    for (int stage = 0; stage < CarRace::NUM_STAGES; stage++) {
        // Wait for start signal from referee
        ops.sem_num = CarRace::SEM_START;
        ops.sem_op = -1;
        ops.sem_flg = 0;
        semop(semid, &ops, 1);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        int raceDelay = CarRace::generateRaceDelay(1000, 5000);
        int stepDelay = raceDelay / CarRace::TRACK_LENGTH;
        
        for (int progress = 0; progress <= CarRace::TRACK_LENGTH; progress++) {
            ProgressMsg msg = {carId, progress};
            msgsnd(progressQueueId, &msg, sizeof(msg) - sizeof(long), 0);
            usleep(stepDelay * 1000);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        double stageTime = std::chrono::duration<double>(endTime - startTime).count();
        ResultMsg resultMsg = {carId, stageTime};
        msgsnd(resultQueueId, &resultMsg, sizeof(resultMsg) - sizeof(long), 0);

        //from car finish the race
        ops.sem_num = CarRace::SEM_FINISH;
        ops.sem_op = 1;
        ops.sem_flg = 0;
        semop(semid, &ops, 1);
    }
}

void runRefereeProcess() {
    std::vector<RaceResult> results(CarRace::NUM_CARS);
    std::vector<int> totalPoints(CarRace::NUM_CARS + 1, 0);
    
    for (int stage = 0; stage < CarRace::NUM_STAGES; stage++) {
        std::vector<int> positions(CarRace::NUM_CARS + 1, 0);
        
        clearScreen();
        std::cout << "\n=== Stage " << stage + 1 << " ===\n\n";
        displayTrack(positions);
        //reset the finish count
        semctl(semid, CarRace::SEM_FINISH, SETVAL, 0);
        
        //from referee start the race
        struct sembuf ops; 
        ops.sem_num = CarRace::SEM_START;
        ops.sem_op = CarRace::NUM_CARS;
        ops.sem_flg = 0;
        semop(semid, &ops, 1);
        
        bool raceComplete = false;
        while (!raceComplete) {
            for (int car = 1; car <= CarRace::NUM_CARS; car++) {
                ProgressMsg msg;
                //from car receive the progress
                if (msgrcv(progressQueueId, &msg, sizeof(msg) - sizeof(long), car, IPC_NOWAIT) != -1) {
                    positions[car] = msg.progress;
                }
            }

            clearScreen();
            std::cout << "\n=== Stage " << stage + 1 << " ===\n\n";
            displayTrack(positions);
            usleep(50000);

            //check if all cars finished the race
            int finishCount = semctl(semid, CarRace::SEM_FINISH, GETVAL, 0);
            if (finishCount >= CarRace::NUM_CARS) {
                raceComplete = true;
            }
        }

        for (int i = 1; i <= CarRace::NUM_CARS; i++) {
            ResultMsg resultMsg;
            
            if (msgrcv(resultQueueId, &resultMsg, sizeof(resultMsg) - sizeof(long), i, 0) == -1) {
                perror("msgrcv failed for result");
                exit(1);
            }
            
            results[i-1].carId = i;
            results[i-1].stageTime = resultMsg.stageTime;
        }

        std::sort(results.begin(), results.end(),
                 [](const RaceResult& a, const RaceResult& b) {
                     return a.stageTime < b.stageTime;
                 });

        std::cout << "\nStage " << stage + 1 << " Results:\n";
        std::cout << "Position | Car ID | Time (s) | Points\n";
        std::cout << "---------|---------|----------|--------\n";

        for (int i = 0; i < CarRace::NUM_CARS; i++) {
            results[i].position = i + 1;
            results[i].points = CarRace::POINTS[i];
            totalPoints[results[i].carId] += results[i].points;

            std::cout << std::setw(9) << results[i].position << "|"
                     << std::setw(9) << results[i].carId << "|"
                     << std::setw(10) << std::fixed << std::setprecision(3) 
                     << results[i].stageTime << "|"
                     << std::setw(8) << results[i].points << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    clearScreen();
    std::cout << "\nFinal Standings:\n";
    std::cout << "Car ID | Total Points\n";
    std::cout << "--------|-------------\n";
    
    std::vector<std::pair<int, int>> standings;
    for (int i = 1; i <= CarRace::NUM_CARS; i++) {
        standings.push_back({i, totalPoints[i]});
    }
    
    std::sort(standings.begin(), standings.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    for (const auto& [carId, points] : standings) {
        std::cout << std::setw(8) << carId << "|"
                 << std::setw(13) << points << "\n";
    }
} 