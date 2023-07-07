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
    bool checkSatisfied = false;
public:
    vector<bool> satisfiedInput;
    bool satisfiable = false;
    vector<int> inv;
    map<string, int> varMap;
#ifdef DBG
    set<string> DC;
#endif
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
    bool isDC(const string &name);
    string getRaw();
    bool solve();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
