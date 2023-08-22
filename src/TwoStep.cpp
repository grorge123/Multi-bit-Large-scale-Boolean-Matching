//
// Created by grorge on 6/25/23.
//
#include <chrono>
#include <utility>
#include "TwoStep.h"
#include "CNF.h"
#include "utility.h"
#include "parser.h"
#include "largeScale.h"

TwoStep ts;
void TwoStep::start() {
    bool optimal = false, timeout = false, projection = false;
    vector<MP> R;
    while (!optimal && !timeout){
        startStatistic("outputSolver");
        MP newPair = outputSolver(projection, R);
        stopStatistic("outputSolver");
        if(!newPair.first.empty()){
            R.push_back(newPair);
        }else{
            // TODO disable projection
            if(!projection && cir1.getOutputNum() != cir2.getOutputNum()){
                projection = true;
            }else{
                if(R.empty()){
                    optimal = true;
                    break;
                }
                outputSolverPop(R);
                projection = false;
            }
            continue;
        }
        if(verbose){
            cout << "Output Pair:" << projection << " " << R.size() << endl;
            for(const auto& pair : R){
                cout << pair.first << ' ' << pair.second << endl;
            }
        }
        clauseNum.push_back(clauseStack.size());
        startStatistic("inputMatch");
        vector<MP> inputMatch = inputSolver(R, projection);
        stopStatistic("inputMatch");
        if(inputMatch.empty()){
            outputSolverPop(R);
        }else{
            int matchOutput = recordOutput(inputMatch, R);
            if( matchOutput == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
        iteratorCounter++;
//        cout << "iteration: " << iteratorCounter << endl;
    }
    cout << "Final iteration: " << iteratorCounter << " " << optimal << endl;
}

int TwoStep::nowMs() {
    return static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
}
int recordCounter = 0;

int TwoStep::recordOutput(const vector<MP> &inputMatch, const vector<MP> &R) {
    unordered_map<string, int> nameMap;
    vector<Group> inputGroups, outputGroups;
    vector<string> one, zero;
    int matchOutput = 0;
    for (const auto &pair: R) {
        if (nameMap.find(pair.first) == nameMap.end()) {
            Group newGroup;
            newGroup.cir1 = pair.first;
            nameMap[pair.first] = outputGroups.size();
            outputGroups.push_back(newGroup);
            matchOutput++;
        }
        outputGroups[nameMap[pair.first]].cir2.push_back(pair.second);
        outputGroups[nameMap[pair.first]].invVector.push_back(false);
        matchOutput++;
    }
    for (const auto &pair: inputMatch) {
        if (pair.first.size() == 1) {
            if (pair.first == "1") {
                one.push_back(pair.second);
            } else {
                zero.push_back(pair.second);
            }
        } else {
            if (nameMap.find(pair.first) == nameMap.end()) {
                Group newGroup;
                newGroup.cir1 = pair.first;
                nameMap[pair.first] = inputGroups.size();
                inputGroups.push_back(newGroup);
            }
            inputGroups[nameMap[pair.first]].cir2.push_back(pair.second);
            inputGroups[nameMap[pair.first]].invVector.push_back(false);
        }
    }
#ifdef INF
    cout << "Two step matching port number: " << matchOutput << "("
         << (float) matchOutput / (float) allOutputNumber * 100 << "%) in " << iteratorCounter
         << " iterations."<< endl;
    cout << "Max match is " << maxMatchNumber << ". time: " << (nowMs() - startMs) / 1000 << endl;
#endif
    parseOutput(outputFilePath, OutputStructure{inputGroups, outputGroups, one, zero}, matchOutput);
    return  matchOutput;
}

