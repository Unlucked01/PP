#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <vector>

enum class FuelType {
    AI_76,
    AI_92,
    AI_95
};

inline std::string getFuelTypeName(FuelType type) {
    switch (type) {
        case FuelType::AI_76: return "AI-76";
        case FuelType::AI_92: return "AI-92";
        case FuelType::AI_95: return "AI-95";
        default: return "Unknown";
    }
}

struct Request {
    int id;
    FuelType fuelType;
    time_t timestamp;
};

class SharedQueue {
public:
    SharedQueue(int maxSize);
    ~SharedQueue();
    
    bool addRequest(const Request& request);
    bool getRequest(int stationId, FuelType stationFuelType, Request& request);
    int getCurrentSize() const;
    void cleanupRemainingRequests();
    
private:
    int shmId;
    int semId;
    struct QueueData* data;
    void lockQueue();
    void unlockQueue();
    void initializeSemaphore();
};