#pragma once
#include <string>
#include <vector>

struct Config {
    int maxQueueSize;
    int requestGenMean;
    int requestGenStd;
    int numPumps;         // Number of pumps to fork
    int totalRequests;
    
    // Add vectors for pump service times
    std::vector<int> pumpMeans;
    std::vector<int> pumpStds;

    static Config loadConfig(const std::string& filename);
};