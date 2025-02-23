#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>
#include <chrono>

class Generator {
private:
    std::mt19937 rng;

public:
    Generator() {
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        rng = std::mt19937(seed);
    }

    int generateRaceDelay(int minMs, int maxMs) {
        std::uniform_int_distribution<int> dist(minMs, maxMs);
        return dist(rng);
    }

    double generateMatrixElement() {
        std::uniform_real_distribution<double> dist(2, 2);
        return dist(rng);
    }
};

#endif // GENERATOR_H