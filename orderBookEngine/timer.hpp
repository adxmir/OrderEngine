#ifndef LATENCY_H
#define LATENCY_H

#include <iostream>
#include <chrono>

namespace Latency{
    class timer{
       std::chrono::high_resolution_clock::time_point start;
        public:
        void startTimer(){
            start = std::chrono::high_resolution_clock::now();
        }

        long long endTimer(){
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            return duration;
        }   
    };
};

#endif
