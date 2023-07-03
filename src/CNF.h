//
// Created by grorge on 6/28/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H

#include <utility>
#include <vector>
#include <string>
#include <map>
#include "AIG.h"
using namespace std;
class CNF {
    vector<int> satisfiedInput;
    bool checkSatisfied = false;
    bool satisfied = false;
public:
    map<string, int> varMap;
    int maxIdx = 0;
    vector<vector<int> > clauses;
    CNF()= default;
    explicit CNF(string inputPath){
        readFromFile(std::move(inputPath));
    }
    explicit CNF(AIG aig){
        readFromAIG(aig);
    }
    void readFromAIG(AIG &aig);
    void readFromFile(string inputPath);
    void combine(const CNF &a);
    string getRow();
    bool isSatisfied();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
