//
// Created by grorge on 6/25/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H

#include <stack>
#include <utility>
#include <functional>
#include "parser.h"
#include "AIG.h"
#include "CNF.h"
typedef pair<string, string> MP;
class TwoStep {
    string outputFilePath;
    AIG cir1, cir2;
    int allOutputNumber{};
    // output solver
    set<size_t> forbid;
    stack<MP> backtrace;
    vector<vector<bool> > initVe;
    vector<int> cir1Choose; // how many number be chosen by cir2 port
    vector<int> cir2Choose; // choose which cir1 port
    vector<string> cir1Output, cir2Output;
    map<string, int> cir1OutputMap, cir2OutputMap;
    int lastCir1, lastCir2;
    vector<vector<string>> clauseStack;
    vector<int> clauseNum;
    // input solver
    int mappingSpaceLast = 0;
    string mappingSpaceFileName = "TwoStepSolveMapping.cnf";

    // hyper parameter
    int maxRunTime = 1000 * 3500; // ms
    bool outputSolverInit = false;
    int iteratorCounter = 0;
    int lastTime = 0;

    void recordMs();
    static int nowMs();
    vector<int> generateOutputGroups(vector<string> &f, vector<string> &g);
    MP outputSolver(bool projection, vector<MP> &R);
    void outputSolverPop();
    vector<MP> inputSolver(vector<MP> &R);
    vector<MP> solveMapping(CNF &mappingSpace, AIG &cir1, AIG &cir2, const int baseLength);
    pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>>, vector<vector<bool>>>
    solveMiter(const vector<MP> &inputMatchPair, const vector<MP> &outputMatchPair, AIG cir1, AIG cir2);
    void reduceSpace(CNF &mappingSpace, const vector<bool> &counter, const int baseLength, AIG &cir1, AIG &cir2,
                     const vector<MP> &mapping, pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>> &nameToOrder
#ifdef DBG
                    , const vector<MP> &R
#endif
                     );
    vector<int> getNonRedundant(const vector<bool> &input, AIG & cir); // return port order
    bool heuristicsOrderCmp(const string& a, const string& b);
    static pair<string, bool> analysisName(string name);
    struct PairHash {
        size_t operator()(const std::pair<std::string, std::string>& p) const {
            return std::hash<std::string>()(p.first) ^ std::hash<std::string>()(p.second);
        }
    };
    struct SetHash {
        size_t operator()(const std::set<std::pair<std::string, std::string>>& s) const {
            size_t seed = 0;
            PairHash ph;
            for (const auto& p : s) {
                seed ^= ph(p) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    static inline size_t hashSet(const std::set<std::pair<std::string, std::string>>& s) {
        SetHash sh;
        return sh(s);
    }


public:
    TwoStep()= default;
    TwoStep(const InputStructure& input, string outputFilePath) : outputFilePath(std::move(outputFilePath)){
        cir1 = AIG(input.cir1AIGPath, "!");
        cir2 = AIG(input.cir2AIGPath, "@");
        allOutputNumber = (cir2.getOutputNum() + cir1.getOutputNum());
    }
    void start();
    void tsDebug(string msg, AIG cir1, AIG cir2);
};

extern TwoStep ts;
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
