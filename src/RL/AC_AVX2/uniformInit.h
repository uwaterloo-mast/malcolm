#ifndef BACKPROPTEST_UNIFORMINIT_H
#define BACKPROPTEST_UNIFORMINIT_H

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>

class UniformInit {
public:
    static void generateData(float* data, size_t dataSize, float variance, float mean=0);
private:
    inline static std::random_device rnd_device {};
    inline static std::mt19937 mersenne_engine {rnd_device()};
};


#endif //BACKPROPTEST_UNIFORMINIT_H
