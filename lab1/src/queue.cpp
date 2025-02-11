#include "queue.h"
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <ctime>
#include "logger.h"

struct QueueData {
    int size;
    int front;
    int rear;
    int maxSize;
    Request requests[1000];
};

SharedQueue::SharedQueue(int maxSize) {
    key_t key = ftok(".", 'Q');
    shmId = shmget(key, sizeof(QueueData), IPC_CREAT | 0666);
    if (shmId == -1) {
        throw std::runtime_error("Failed to create shared memory");
    }

    data = (QueueData*)shmat(shmId, nullptr, 0);
    if (data == (void*)-1) {
        throw std::runtime_error("Failed to attach shared memory");
    }

    data->size = 0;
    data->front = 0;
    data->rear = -1;
    data->maxSize = maxSize;

    initializeSemaphore();
}

SharedQueue::~SharedQueue() {
    shmdt(data);
    shmctl(shmId, IPC_RMID, nullptr);
    semctl(semId, 0, IPC_RMID);
}

void SharedQueue::initializeSemaphore() {
    key_t key = ftok(".", 'S');
    semId = semget(key, 1, IPC_CREAT | 0666);
    if (semId == -1) {
        throw std::runtime_error("Failed to create semaphore");
    }
    
    semctl(semId, 0, SETVAL, 1);
}

void SharedQueue::lockQueue() {
    struct sembuf sb = {0, -1, 0};
    semop(semId, &sb, 1);
}

void SharedQueue::unlockQueue() {
    struct sembuf sb = {0, 1, 0};
    semop(semId, &sb, 1);
}

bool SharedQueue::addRequest(const Request& request) {
    lockQueue();
    bool success = false;
    
    if (data->size < data->maxSize) {
        data->rear = (data->rear + 1) % data->maxSize;
        data->requests[data->rear] = request;
        data->size++;
        success = true;
        
        #ifdef DEBUG
        std::ofstream debugLog("logs/debug.log", std::ios::app);
        debugLog << "Added Request " << request.id << " to queue (size=" << data->size << ")\n";
        debugLog << "Queue after addition:\n";
        for (int i = 0; i < data->size; i++) {
            int idx = (data->front + i) % data->maxSize;
            debugLog << "  [" << i << "] Request " << data->requests[idx].id 
                    << " (fuel: " << getFuelTypeName(data->requests[idx].fuelType) << ")\n";
        }
        debugLog << "-------------------\n";
        debugLog.close();
        #endif
    }
    
    unlockQueue();
    return success;
}

bool SharedQueue::getRequest(int stationId, FuelType stationFuelType, Request& request) {
    lockQueue();
    bool found = false;
    
    if (data->size > 0) {
        // Search for a matching fuel type request
        int matchIndex = -1;
        
        // Debug log current queue state
        #ifdef DEBUG
        std::ofstream debugLog("logs/debug.log", std::ios::app);
        debugLog << "Station " << stationId << " checking queue (size=" << data->size << "):\n";
        for (int i = 0; i < data->size; i++) {
            int idx = (data->front + i) % data->maxSize;
            debugLog << "  [" << i << "] Request " << data->requests[idx].id 
                    << " (fuel: " << getFuelTypeName(data->requests[idx].fuelType) << ")\n";
        }
        debugLog.close();
        #endif

        for (int i = 0; i < data->size; i++) {
            int idx = (data->front + i) % data->maxSize;
            if (data->requests[idx].fuelType == stationFuelType) {
                matchIndex = i;
                break;
            }
        }
        
        if (matchIndex != -1) {
            int idx = (data->front + matchIndex) % data->maxSize;
            request = data->requests[idx];
            
            for (int i = matchIndex; i < data->size - 1; i++) {
                int curr = (data->front + i) % data->maxSize;
                int next = (data->front + i + 1) % data->maxSize;
                data->requests[curr] = data->requests[next];
            }
            
            data->size--;
            if (data->size == 0) {
                data->front = 0;
                data->rear = -1;
            } else {
                data->rear = (data->front + data->size - 1) % data->maxSize;
            }
            
            found = true;

            #ifdef DEBUG
            std::ofstream debugLog("logs/debug.log", std::ios::app);
            debugLog << "Station " << stationId << " removed Request " << request.id 
                    << " (remaining size=" << data->size << ")\n";
            debugLog << "Queue after removal:\n";
            for (int i = 0; i < data->size; i++) {
                int idx = (data->front + i) % data->maxSize;
                debugLog << "  [" << i << "] Request " << data->requests[idx].id 
                        << " (fuel: " << getFuelTypeName(data->requests[idx].fuelType) << ")\n";
            }
            debugLog << "-------------------\n";
            debugLog.close();
            #endif
        }
    }
    
    unlockQueue();
    return found;
}

void SharedQueue::cleanupRemainingRequests() {
    lockQueue();
    
    if (data->size > 0) {
        std::ofstream rejectedLog("logs/rejected.log", std::ios::app);
        
        #ifdef DEBUG
        std::ofstream debugLog("logs/debug.log", std::ios::app);
        debugLog << "\nRemaining requests at shutdown:\n";
        #endif
        
        for (int i = 0; i < data->size; i++) {
            int idx = (data->front + i) % data->maxSize;
            Request& request = data->requests[idx];
            
            Logger::logRejected(rejectedLog, request.id,
                              getFuelTypeName(request.fuelType),
                              data->size,
                              " (shutdown)");
            
            #ifdef DEBUG
            debugLog << "  [" << i << "] Request " << request.id 
                    << " (fuel: " << getFuelTypeName(request.fuelType) 
                    << ") moved to rejected\n";
            #endif
        }
        
        #ifdef DEBUG
        debugLog << "-------------------\n";
        debugLog.close();
        #endif
        
        rejectedLog.close();
        
        // Clear the queue
        data->size = 0;
        data->front = 0;
        data->rear = -1;
    }
    
    unlockQueue();
}

int SharedQueue::getCurrentSize() const {
    return data->size;
}
