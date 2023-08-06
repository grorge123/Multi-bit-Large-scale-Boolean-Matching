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
    int maxIdx = 0;
    CNF()= default;
    CNF(const CNF& other) {
        // Copy all other members
        change = 2;
        lastClauses = 0;
        clauses = other.clauses;
        satisfiedInput = other.satisfiedInput;
        satisfiable = other.satisfiable;
        inv = other.inv;
        varMap = other.varMap;
        maxIdx = other.maxIdx;
        solver = new CaDiCaL::Solver();
    }

    // Assignment operator
    CNF& operator=(const CNF& other) {
        if (this != &other) {
            delete solver;
            change = 2;
            lastClauses = 0;
            clauses = other.clauses;
            satisfiedInput = other.satisfiedInput;
            satisfiable = other.satisfiable;
            inv = other.inv;
            varMap = other.varMap;
            maxIdx = other.maxIdx;
            solver = new CaDiCaL::Solver();
        }
        return *this;
    }
    ~CNF() {
        delete solver;
    }
    explicit CNF(string inputPath){
        readFromFile(std::move(inputPath));
    }
    explicit CNF(AIG aig){
        readFromAIG(aig);
    }
    list<vector<int>>::iterator addClause(const vector<int> &clause);
    void eraseClause(list<vector<int>>::iterator it);
    void readFromAIG(AIG &aig);
    void readFromFile(const string& inputPath);
    void combine(const CNF &a);
    const vector<int> &getClause(list<vector<int>>::iterator it);
    void addAssume(int lit);
    string getRaw();
    void copy(CNF &other); // copy this to other
    bool solve();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_CNF_H
