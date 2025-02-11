#pragma once
#include <string>
#include <vector>
#include "queue.h"

struct Config {
    int maxQueueSize;
    int requestGenMean;
    int requestGenStd;
    int numPumps;
    int totalRequests;
    
    std::vector<int> pumpMeans;
    std::vector<int> pumpStds;
    std::vector<FuelType> pumpFuelTypes;

    static Config loadConfig(const std::string& filename);
};