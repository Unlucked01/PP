#include "car_race.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>


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

void runCarProcess(int rank) {
    RaceResult result;
    result.carId = rank;

    for (int stage = 0; stage < CarRace::NUM_STAGES; stage++) {
        MPI_Barrier(MPI_COMM_WORLD);
        
        double startTime = MPI_Wtime();
        int raceDelay = CarRace::generateRaceDelay(1000, 5000);
        
        int stepDelay = raceDelay / CarRace::TRACK_LENGTH;
        
        for (int progress = 0; progress <= CarRace::TRACK_LENGTH; progress++) {
            MPI_Send(&progress, 1, MPI_INT, CarRace::REFEREE_RANK, 
                    CarRace::PROGRESS_TAG, MPI_COMM_WORLD);
            
            usleep(stepDelay * 1000);
        }
        
        double endTime = MPI_Wtime();
        result.stageTime = endTime - startTime;
        
        MPI_Barrier(MPI_COMM_WORLD);
        
        MPI_Send(&result, sizeof(RaceResult), MPI_BYTE, 
                 CarRace::REFEREE_RANK, CarRace::RESULT_TAG, MPI_COMM_WORLD);
    }
}

void runRefereeProcess() {
    std::vector<RaceResult> results(CarRace::NUM_CARS);
    std::vector<int> totalPoints(CarRace::NUM_CARS + 1, 0);
    std::vector<int> positions(CarRace::NUM_CARS + 1, 0);

    for (int stage = 0; stage < CarRace::NUM_STAGES; stage++) {
        clearScreen();
        std::cout << "\n=== Stage " << stage + 1 << " ===\n\n";
        
        std::fill(positions.begin(), positions.end(), 0);
        MPI_Barrier(MPI_COMM_WORLD);
        
        bool raceComplete = false;
        while (!raceComplete) {
            raceComplete = true;
            
            for (int car = 1; car <= CarRace::NUM_CARS; car++) {
                int flag;
                MPI_Status status;
                MPI_Iprobe(car, CarRace::PROGRESS_TAG, MPI_COMM_WORLD, &flag, &status);
                
                if (flag) {
                    int progress;
                    MPI_Recv(&progress, 1, MPI_INT, car, CarRace::PROGRESS_TAG, 
                            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    positions[car] = progress;
                }
                
                if (positions[car] < CarRace::TRACK_LENGTH) {
                    raceComplete = false;
                }
            }

            clearScreen();
            std::cout << "\n=== Stage " << stage + 1 << " ===\n\n";
            displayTrack(positions);
            usleep(50000); // 50ms delay for visualization
        }
        
        MPI_Barrier(MPI_COMM_WORLD);

        for (int i = 1; i <= CarRace::NUM_CARS; i++) {
            MPI_Recv(&results[i-1], sizeof(RaceResult), MPI_BYTE, 
                    i, CarRace::RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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