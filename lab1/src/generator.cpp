#include "generator.h"
#include "logger.h"
#include <random>
#include <chrono>
#include <fstream>
#include <signal.h>
#include <thread>

static bool running = true;

void handleSignalGenerator(int) {
    running = false;
}

RequestGenerator::RequestGenerator(SharedQueue& q, const Config& c)
    : queue(q), config(c) {
    std::ofstream logFile("logs/queue.log", std::ios::trunc);
    logFile.close();
    
    std::ofstream rejectedLog("logs/rejected.log", std::ios::trunc);
    rejectedLog.close();
}

void RequestGenerator::run() {
    signal(SIGTERM, handleSignalGenerator);
    generateRequests();
}

FuelType RequestGenerator::getRandomFuelType() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 2);
    
    switch (dis(gen)) {
        case 0: return FuelType::AI_76;
        case 1: return FuelType::AI_92;
        default: return FuelType::AI_95;
    }
}

void RequestGenerator::generateRequests() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> delay_dist(config.requestGenMean, config.requestGenStd);
    
    int requestId = 0;
    
    while (running && requestId < config.totalRequests) {
        Request request;
        request.id = ++requestId;
        request.fuelType = getRandomFuelType();
        request.timestamp = std::time(nullptr);
        
        std::string timestamp = std::ctime(&request.timestamp);
        timestamp = timestamp.substr(0, timestamp.length() - 1);
        
        if (queue.addRequest(request)) {
            std::ofstream logFile("logs/queue.log", std::ios::app);
            Logger::logGeneration(logFile, request.id, 
                                getFuelTypeName(request.fuelType), 
                                timestamp);
            logFile.close();
        } else {
            std::ofstream rejectedLog("logs/rejected.log", std::ios::app);
            
            Logger::logRejected(rejectedLog, request.id,
                              getFuelTypeName(request.fuelType),
                              queue.getCurrentSize(),
                              timestamp);
            
            rejectedLog.close();
        }
        
        int delay = std::max(100, static_cast<int>(delay_dist(gen)));
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}