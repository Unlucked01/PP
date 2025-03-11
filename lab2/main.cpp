#include "car_race.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

int main() {
    initializeIPC();
    
    std::vector<pid_t> carProcesses;
    
    for (int i = 1; i <= CarRace::NUM_CARS; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            runCarProcess(i);
            exit(0);
        } else {
            carProcesses.push_back(pid);
        }
    }
    
    runRefereeProcess();
    
    for (pid_t pid : carProcesses) {
        waitpid(pid, NULL, 0);
    }
    
    cleanupIPC();
    
    return 0;
} 