//
// Created by grorge on 6/25/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H

#include <stack>
#include "parser.h"
#include "AIG.h"
typedef pair<string, string> MP;
class TwoStep {
    string outputFilePath;
    AIG cir1, cir2;
    int allOutputNumber;
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

    template <typename T>
    struct VectorHash {
        template <typename U = T>
        std::size_t calHash(const U& a,
                            typename std::enable_if<!std::is_same<U, std::pair<std::string, std::string>>::value>::type* = nullptr) const {
            return std::hash<U>{}(a);
        }
        size_t calHash(pair<string, string> p) const {
            std::size_t hash = std::hash<std::string>{}(p.first);
            hash ^= std::hash<std::string>{}(p.second) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
        size_t doHash(const vector<T>& vec) const {
            size_t seed = vec.size();
            for (const auto& elem : vec) {
                seed ^= calHash(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    static int nowMs();
    vector<int> generateOutputGroups(vector<string> &f, vector<string> &g);
    MP outputSolver(bool projection);
    void outputSolverPop();
    bool inputSolver();
    bool heuristicsOrderCmp(string a, string b);
public:
    TwoStep(){

    }
    TwoStep(InputStructure input, string outputFilePath) : outputFilePath(outputFilePath){
        cir1 = AIG(input.cir1AIGPath, "!");
        cir2 = AIG(input.cir2AIGPath, "@");
        allOutputNumber = (cir2.getOutputNum() + cir1.getOutputNum());
    }
    void start();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
