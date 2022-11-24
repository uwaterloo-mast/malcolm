#include "uniformInit.h"

void UniformInit::generateData(float* data, size_t dataSize, float variance, float mean){
    std::uniform_real_distribution<> dist {mean - 1.0/sqrt(variance), mean + 1.0/sqrt(variance)};
    auto gen = [&dist](){
        return dist(mersenne_engine);
    };
    std::generate(data, data + dataSize, gen);
}

