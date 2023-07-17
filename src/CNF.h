//
// Created by grorge on 6/28/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H

#include <utility>
#include <list>
#include <vector>
#include <string>
#include <map>
#include "AIG.h"
#include "cadical.hpp"
using namespace std;
class CNF {
    CaDiCaL::Solver *solver = new CaDiCaL::Solver();
    int change = 1;
    int lastClauses = 0;
    list<vector<int> > clauses;
public:
    const list <vector<int>> &getClauses() const;
    vector<bool> satisfiedInput;
    bool satisfiable = false;
    vector<int> inv;
    map<string, int> varMap;
#ifdef DBG
    set<string> DC;
#endif
    int maxIdx = 0;
    CNF()= default;
    explicit CNF(string inputPath){
        readFromFile(std::move(inputPath));
    }
    explicit CNF(AIG aig){
        readFromAIG(aig);
    }
    list<vector<int>>::iterator addClause(const vector<int> &clause);
    void readFromAIG(AIG &aig);
    void readFromFile(string inputPath);
    void combine(const CNF &a);
    bool isDC(const string &name);
    string getRaw();
    bool solve();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
