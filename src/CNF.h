//
// Created by grorge on 6/28/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H

#include <vector>
#include <string>
#include "AIG.h"
using namespace std;

class CNF {
    int maxIdx = 0;
    vector<vector<int> > clauses;
    vector<int> satisfiedInput;
    map<string, int> varMap;
    bool checkSatisfied = false;
    bool satisfied = false;
    CNF(){

    }
    CNF(string inputPath){
        readFromFile(inputPath);
    }
    CNF(AIG aig){
        readFromAIG(aig);
    }
    void readFromAIG(AIG &aig);
    void readFromFile(string inputPath);
    void combine(const CNF &a);
    string getRow();
    bool isSatisfied();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
