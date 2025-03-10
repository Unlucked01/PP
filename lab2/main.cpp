#include "car_race.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

int main() {
    // Initialize IPC resources
    initializeIPC();
    
    std::vector<pid_t> carProcesses;
    
    // Create car processes
    for (int i = 1; i <= CarRace::NUM_CARS; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            cleanupIPC();
            exit(1);
        } else if (pid == 0) {
            // Child process (car)
            runCarProcess(i);
            exit(0);
        } else {
            // Parent process
            carProcesses.push_back(pid);
        }
    }
    
    // Parent process becomes the referee
    runRefereeProcess();
    
    // Wait for all car processes to finish
    for (pid_t pid : carProcesses) {
        waitpid(pid, NULL, 0);
    }
    
    // Clean up IPC resources
    cleanupIPC();
    
    return 0;
} 