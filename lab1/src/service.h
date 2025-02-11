#pragma once
#include "queue.h"
#include "config.h"
#include <string>

class ServiceStation {
public:
    ServiceStation(SharedQueue& queue, int stationId, const Config& config);
    void run();
    
private:
    SharedQueue& queue;
    int stationId;
    const Config& config;
    std::string logFile;
    
    void logService(const Request& request);
};