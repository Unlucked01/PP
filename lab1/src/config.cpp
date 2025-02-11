#include "config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

Config Config::loadConfig(const std::string& filename) {
    Config config;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string key;
        std::getline(iss, key, '=');
        
        int value;
        iss >> value;

        if (key == "MAX_QUEUE_SIZE") config.maxQueueSize = value;
        else if (key == "REQUEST_GEN_MEAN") config.requestGenMean = value;
        else if (key == "REQUEST_GEN_STD") config.requestGenStd = value;
        else if (key == "TOTAL_REQUESTS") config.totalRequests = value;
        else if (key.find("PUMP") != std::string::npos && key.find("MEAN") != std::string::npos) {
            config.pumpMeans.push_back(value);
        }
        else if (key.find("PUMP") != std::string::npos && key.find("STD") != std::string::npos) {
            config.pumpStds.push_back(value);
        }
    }

    config.numPumps = config.pumpMeans.size();
    return config;
}