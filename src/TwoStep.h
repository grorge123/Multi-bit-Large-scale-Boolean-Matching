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
typedef pair<int, int> pii;
class TwoStep {
    string outputFilePath;
    AIG cir1, cir2;
    int allOutputNumber{};
    int startMs = nowMs();
    // output solver
    vector<int> hGroupId;
    set<size_t> forbid;
    stack<MP> backtrace;
    vector<vector<bool> > initVe;
    vector<int> cir1Choose; // how many number be chosen by cir2 port
    vector<int> cir2Choose; // choose which cir1 port
    vector<string> cir1Output, cir2Output;
    unordered_map<string, int> cir1OutputMap, cir2OutputMap;
    vector<int> cir1OutputMaxSup;
    vector<int> cir2OutputMaxSup;

//    int lastCir1, lastCir2;
    vector<vector<string>> clauseStack;
    vector<int> clauseNum;
    // Bus
    unordered_map<int ,int> cir1OutputBusMatch, cir2OutputBusMatch;
    unordered_map<string, int> cir1BusMapping, cir2BusMapping;
    vector<vector<string> > cir1InputBus, cir2InputBus;
    vector<vector<string> > cir1OutputBus, cir2OutputBus;
    // input solver
    string mappingSpaceFileName = "TwoStepSolveMapping.cnf";
    // hyper parameter
    int maxRunTime = 1000 * 3500; // ms
//    int maxRunTime = 1000 * 10; // ms
    bool outputSolverInit = false;
    int iteratorCounter = 0;
    int lastTime = 0;
    int verbose = 1;
    bool enableOutputBus = true;
    bool enableInputBus = true;

    static int nowMs();
    vector<int> generateOutputGroups(vector<string> &f, vector<string> &g);
    MP outputSolver(bool projection, vector<MP> &R);
    void outputSolverPop(vector<MP> &R);
    int recordOutput(const vector<MP> &inputMatch, const vector<MP> &R);
public:
    vector<MP> inputSolver(vector<MP> &R, bool outputProjection);
private:
    bool generateClause(CNF &mappingSpace, AIG &cir1Reduce, AIG &cir2Reduce, const vector<MP> &R,
                        bool outputProjection);
    void generateBusClause(CNF &mappingSpace, AIG &cir1Reduce, AIG &cir2Reduce, const vector<int> &cir1BusMatch,
                           const vector<int> &cir2BusMatch, const int lastMaxIdx, const vector<MP> &R);
    static vector<MP> solveMapping(CNF &mappingSpace, AIG &cir1, AIG &cir2, const int baseLength);
    CNF generateMiter(const vector<MP> &outputMatchPair, AIG cir1, AIG cir2);
    pair<vector<bool>, vector<bool>>
    solveMiter(const vector<MP> &inputMatchPair, CNF originMiter, AIG &cir1, AIG &cir2);
    pair<pair<vector<int>, unordered_map<int, int>>, pair<vector<int>, unordered_map<int, int>>>
    generateBusMatchVector(AIG &cir1, AIG &cir2);
    void reduceSpace(CNF &mappingSpace, const int baseLength, AIG &cir1, AIG &cir2, const vector<MP> &mapping,
                     const vector<MP> &R, const vector<bool> &cir1Input, const vector<bool> &cir2Input);
    static vector<int> getNonRedundant(const vector<bool> &input, AIG &cir, int counterIdx); // return port order
    static vector<int> getNonRedundant2(const vector<bool> &input, AIG &cir, int counterIdx); // return port order
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
//        cir1.optimize();
        cir2 = AIG(input.cir2AIGPath, "@");
//        cir2.optimize();
        allOutputNumber = (cir2.getOutputNum() + cir1.getOutputNum());
        for(auto &bus: input.cir1Bus){
            if(cir1.isInput("!" + bus[0])){
                cir1InputBus.emplace_back(bus);
            }else{
                cir1OutputBus.emplace_back(bus);
            }
        }
        for(auto &bus: input.cir2Bus){
            if(cir2.isInput("@" + bus[0])){
                cir2InputBus.emplace_back(bus);
            }else{
                cir2OutputBus.emplace_back(bus);
            }
        }
        for(auto &bus : cir1InputBus){
            for(auto &name : bus){
                name.insert(0, "!");
            }
        }
        for(auto &bus : cir2InputBus){
            for(auto &name : bus){
                name.insert(0, "@");
            }
        }
        for(auto &bus : cir1OutputBus){
            for(auto &name : bus){
                name.insert(0, "!");
            }
        }
        for(auto &bus : cir2OutputBus){
            for(auto &name : bus){
                name.insert(0, "@");
            }
        }
        for(int i = 0 ; i < static_cast<int>(cir1InputBus.size()) ; i++){
            for(const auto& name : cir1InputBus[i]){
                cir1BusMapping[name] = i;
            }
        }
        for(int i = 0 ; i < static_cast<int>(cir2InputBus.size()) ; i++){
            for(const auto& name : cir2InputBus[i]){
                cir2BusMapping[name] = i;
            }
        }
        for(int i = 0 ; i < static_cast<int>(cir1OutputBus.size()) ; i++){
            for(const auto& name : cir1OutputBus[i]){
                cir1BusMapping[name] = i;
            }
        }
        for(int i = 0 ; i < static_cast<int>(cir2OutputBus.size()) ; i++){
            for(const auto& name : cir2OutputBus[i]){
                cir2BusMapping[name] = i;
            }
        }
        if(verbose){
            cout << "Basic Information" << endl;
            cout << "Input Num:" << cir1.getInputNum() << " " << cir2.getInputNum() << endl;
            cout << "Output Num:" << cir1.getOutputNum() << " " << cir2.getOutputNum() << endl;
        }
#ifdef  DBG
//        for(int i = 0 ; i < cir1.getInputNum() ; i++){
//            string name = cir1.fromOrderToName(i);
//            if(cir1BusMapping.find(name) == cir1BusMapping.end()){
//                cir1BusMapping[name] = -1;
//            }
//        }
//        for(int i = 0 ; i < cir2.getInputNum() ; i++){
//            string name = cir2.fromOrderToName(i);
//            if(cir2BusMapping.find(name) == cir2BusMapping.end()){
//                cir2BusMapping[name] = -1;
//            }
//        }
#endif
        int cnt = 0;
        cout << "Bus1:" << endl;
        for(auto &bus : cir1OutputBus){
            cout << cnt << ": ";
            cnt++;
            for(auto &name : bus){
                cout << name << " ";
            }
            cout << endl;
        }
        cnt = 0;
        cout << "Bus2:" << endl;
        for(auto &bus : cir2OutputBus){
            cout << cnt << ": ";
            cnt++;
            for(auto &name : bus){
                cout << name << " ";
            }
            cout << endl;
        }

        cir1OutputMaxSup.resize(cir1.getOutputNum());
        cir2OutputMaxSup.resize(cir2.getOutputNum());

        for (int i = 0; i < cir1.getOutputNum(); i++){
            auto funSup = cir1.getSupport(cir1.fromOrderToName(cir1.getInputNum() + i), 1);
            for(const auto& sup : funSup){
                cir1OutputMaxSup[i] = max(cir1OutputMaxSup[i], static_cast<int>(cir1.getSupport(sup, 1).size()));
            }
        }
        for (int i = 0; i < cir2.getOutputNum(); i++){
            auto funSup = cir2.getSupport(cir2.fromOrderToName(cir2.getInputNum() + i), 1);
            for(const auto& sup : funSup){
                cir2OutputMaxSup[i] = max(cir2OutputMaxSup[i], static_cast<int>(cir2.getSupport(sup, 1).size()));
            }
        }
    }
    void start();
    void tsDebug(string msg, AIG cir1, AIG cir2);
    void tsDebug(const vector<int> &cir1BusMatch, const vector<int> &cir2BusMatch, const int lastMaxIdx);
};
extern int cnt;
extern TwoStep ts;
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
