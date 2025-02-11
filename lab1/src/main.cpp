#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <filesystem>
#include <signal.h>

#include "config.h"
#include "queue.h"
#include "service.h"
#include "generator.h"


int main() {
    try {
        std::filesystem::create_directory("logs");
        Config config = Config::loadConfig("config.txt");
        
        #ifdef DEBUG
        std::cout << "Starting gas station simulation in DEBUG mode" << std::endl;
        #endif

        SharedQueue queue(config.maxQueueSize);
        std::vector<pid_t> servicePids;
        
        for (int i = 0; i < config.numPumps; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                ServiceStation station(queue, i + 1, config);
                station.run();
                exit(0);
            } else if (pid > 0) {
                servicePids.push_back(pid);
            } else {
                throw std::runtime_error("Fork failed");
            }
        }
        
        pid_t generatorPid = fork();
        if (generatorPid == 0) {
            RequestGenerator generator(queue, config);
            generator.run();
            exit(0);
        } else if (generatorPid < 0) {
            throw std::runtime_error("Fork failed");
        }
        
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        
        kill(generatorPid, SIGTERM);
        for (pid_t pid : servicePids) {
            kill(pid, SIGTERM);
        }
        
        while (wait(nullptr) > 0);
        
        std::cout << "Simulation completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}