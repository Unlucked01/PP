#include "service.h"
#include "logger.h"
#include <random>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <thread>

static bool running = true;

void handleSignal(int) {
    running = false;
}

ServiceStation::ServiceStation(SharedQueue& q, int id, const Config& c)
    : queue(q), stationId(id), config(c) {
    fuelType = config.pumpFuelTypes[id - 1];
    std::ostringstream oss;
    oss << "logs/station_" << stationId << ".log";
    logFile = oss.str();
    std::ofstream log(logFile, std::ios::trunc);
    log.close();
}

void ServiceStation::run() {
    signal(SIGTERM, handleSignal);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int pumpIndex = stationId - 1;
    std::normal_distribution<> service_time(
        config.pumpMeans[pumpIndex],
        config.pumpStds[pumpIndex]
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(50 * stationId));
    
    while (running) {
        Request request;
        if (queue.getRequest(stationId, fuelType, request)) {
            std::string timestamp = std::ctime(&request.timestamp);
            timestamp = timestamp.substr(0, timestamp.length() - 1);
            
            std::ofstream log(logFile, std::ios::app);
            Logger::logQueueRemoval(log, stationId, request.id,
                                  getFuelTypeName(request.fuelType),
                                  queue.getCurrentSize(),
                                  timestamp);
            
            int serviceDelay = std::max(100, static_cast<int>(service_time(gen)));
            std::this_thread::sleep_for(std::chrono::milliseconds(serviceDelay));
            
            timestamp = std::ctime(&request.timestamp);
            timestamp = timestamp.substr(0, timestamp.length() - 1);
            
            Logger::logService(log, stationId, request.id,
                             getFuelTypeName(request.fuelType),
                             timestamp);
            log.close();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}