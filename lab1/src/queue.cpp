#include "queue.h"
#include <cstring>
#include <stdexcept>

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
    }
    
    unlockQueue();
    return success;
}

bool SharedQueue::getRequest(int stationId, Request& request) {
    lockQueue();
    bool found = false;
    
    if (data->size > 0) {
        request = data->requests[data->front];
        
        for (int i = 0; i < data->size - 1; i++) {
            int idx = (data->front + i) % data->maxSize;
            int next_idx = (data->front + i + 1) % data->maxSize;
            data->requests[idx] = data->requests[next_idx];
        }
        
        data->size--;
        if (data->size == 0) {
            data->front = 0;
            data->rear = -1;
        }
        found = true;
    }
    
    unlockQueue();
    return found;
}

int SharedQueue::getCurrentSize() const {
    return data->size;
}