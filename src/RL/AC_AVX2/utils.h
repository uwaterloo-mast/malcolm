#ifndef DLB_AC_UTILS_H
#define DLB_AC_UTILS_H

#include <immintrin.h>
#include <iostream>
#include <string>
#include <sstream>

template <typename T>
std::string __m256_toString(const __m256 var) {
    std::stringstream sstr;
    T values[32/sizeof(T)];
    std::memcpy(values,&var,sizeof(values)); //See discussion below
    if (sizeof(T) == 1) {
        for (unsigned int i = 0; i < sizeof(__m256); i++) { //C++11: Range for also possible
            sstr << (int) values[i] << " ";
        }
    } else {
        for (unsigned int i = 0; i < sizeof(__m256) / sizeof(T); i++) { //C++11: Range for also possible
            sstr << values[i] << " ";
        }
    }
    return sstr.str();
}


#endif