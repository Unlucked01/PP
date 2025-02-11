#pragma once
#include "queue.h"
#include "config.h"

class RequestGenerator {
public:
    RequestGenerator(SharedQueue& queue, const Config& config);
    void run();
    
private:
    SharedQueue& queue;
    const Config& config;
    
    void generateRequests();
    FuelType getRandomFuelType();
};