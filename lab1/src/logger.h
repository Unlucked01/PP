#pragma once
#include <string>
#include <fstream>
#include <iostream>

// ANSI color codes for terminal output
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string BLUE    = "\033[34m";
    const std::string YELLOW  = "\033[33m";
}

class Logger {
public:
    static void logGeneration(std::ofstream& file, int requestId, const std::string& fuelType, const std::string& timestamp) {
        std::string prefix = "[GEN]";
        std::string msg = " Request " + std::to_string(requestId) + 
                         " generated for fuel type " + fuelType + 
                         " at " + timestamp;
        
        file << prefix << msg << std::endl;
        
        #ifdef DEBUG
        std::cout << Color::GREEN << prefix << Color::RESET << msg << std::endl;
        #endif
    }
    
    static void logService(std::ofstream& file, int stationId, int requestId, 
                          const std::string& fuelType, const std::string& timestamp) {
        std::string prefix = "[SERV]";
        std::string msg = " Request " + std::to_string(requestId) + 
                         " serviced by station " + std::to_string(stationId) +
                         " (fuel type: " + fuelType + ") at " + timestamp;
        
        file << prefix << msg << std::endl;
        
        #ifdef DEBUG
        std::cout << Color::YELLOW << prefix << Color::RESET << msg << std::endl;
        #endif
    }
    
    static void logQueueRemoval(std::ofstream& file, int stationId, int requestId, 
                               const std::string& fuelType, int queueSize, const std::string& timestamp) {
        std::string prefix = "[QUEUE]";
        std::string msg = " Request " + std::to_string(requestId) + 
                         " removed by station " + std::to_string(stationId) +
                         " (fuel type: " + fuelType + ") from queue. " +
                         "(" + std::to_string(queueSize) + ")" +
                         " at " + timestamp;
        
        file << prefix << msg << std::endl;
        
        #ifdef DEBUG
        std::cout << Color::BLUE << prefix << Color::RESET << msg << std::endl;
        #endif
    }
    
    static void logRejected(std::ofstream& file, int requestId, 
                           const std::string& fuelType, int queueSize, 
                           const std::string& timestamp) {
        std::string prefix = "[REJECT]";
        std::string msg = " Request " + std::to_string(requestId) + 
                         " rejected (fuel type: " + fuelType + "). " +
                         "Queue is full (" + std::to_string(queueSize) + ") " +
                         "at " + timestamp;
        
        file << prefix << msg << std::endl;
        
        #ifdef DEBUG
        std::cout << Color::RED << prefix << Color::RESET << msg << std::endl;
        #endif
    }
};