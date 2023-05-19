//
// Created by Han Wen Tsao on 2023/5/19.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H

#include <string>
#include <random>
#include "Partition.h"
#include "parser.h"
using namespace std;
class LargeScale {
    default_random_engine generator;
    uniform_int_distribution<int> distribution;
    Partition cir1, cir2;
public:
    LargeScale(inputStructure input) : generator(7122), distribution(0,1){
        //TODO assert IO port equal
        cir1 = Partition(input.cir1AIGPath);
        cir2 = Partition(input.cir2AIGPath);
    }
    void randomSimulation(int only = 0);
    vector<bool> generateInput(int inputNum);
};

#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
