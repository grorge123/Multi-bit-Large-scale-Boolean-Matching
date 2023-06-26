//
// Created by grorge on 6/25/23.
//
#include <chrono>
#include "TwoStep.h"


void TwoStep::start() {
    int startMs = nowMs();
    bool optimal = false, timeout = false, projection = false;
    vector<MP> R;
    set<size_t> forbid;
    while (!optimal && !timeout){
        vector<MP> newPair = outputSolver();
        if(newPair.size() != 0){
            R.push_back(newPair.front());
        }else{
            if(projection){
                projection = true;
            }else{
                forbid.insert(VectorHash<MP>().doHash(R));
                R.pop_back();
                projection = false;
            }
            continue;
        }
        bool satisfied = inputSolver();
        if(satisfied){
            forbid.insert(VectorHash<MP>().doHash(R));
            R.pop_back();
            if(R.size() * 2 == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
    }
}

int TwoStep::nowMs() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}
