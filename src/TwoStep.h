//
// Created by grorge on 6/25/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H

#include <stack>
#include <utility>
#include "parser.h"
#include "AIG.h"
typedef pair<string, string> MP;
class TwoStep {
    string outputFilePath;
    AIG cir1, cir2;
    int allOutputNumber{};
    // output solver
    stack<MP> backtrace;
    vector<vector<bool> > initVe;
    vector<int> cir1Choose; // how many number be chosen by cir2 port
    vector<int> cir2Choose; // choose which cir1 port
    vector<string> cir1Output, cir2Output;
    map<string, int> cir1OutputMap, cir2OutputMap;

    // hyper parameter
    int maxRunTime = 1000 * 3500; // ms
    bool outputSolverInit = false;


    static int nowMs();
    vector<int> generateOutputGroups(vector<string> &f, vector<string> &g);
    MP outputSolver(bool projection);
    void outputSolverPop();
    bool inputSolver();
    bool heuristicsOrderCmp(const string& a, const string& b);
public:
    TwoStep()= default;
    TwoStep(const InputStructure& input, string outputFilePath) : outputFilePath(std::move(outputFilePath)){
        cir1 = AIG(input.cir1AIGPath, "!");
        cir2 = AIG(input.cir2AIGPath, "@");
        allOutputNumber = (cir2.getOutputNum() + cir1.getOutputNum());
    }
    void start();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
