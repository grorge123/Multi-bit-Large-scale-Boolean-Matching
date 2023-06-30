//
// Created by grorge on 6/25/23.
//
#include <chrono>
#include <functional>
#include "TwoStep.h"


void TwoStep::start() {
    int startMs = nowMs();
    bool optimal = false, timeout = false, projection = false;
    vector<MP> R;
    set<size_t> forbid;
    while (!optimal && !timeout){
        MP newPair = outputSolver(projection);
        if(newPair.first != ""){
            R.push_back(newPair);
        }else{
            if(!projection){
                projection = true;
            }else{
                forbid.insert(VectorHash<MP>().doHash(R));
                R.pop_back();
                outputSolverPop();
                projection = false;
            }
            continue;
        }
        bool satisfied = inputSolver();
        if(satisfied){
            forbid.insert(VectorHash<MP>().doHash(R));
            R.pop_back();
            outputSolverPop();
            if(static_cast<int>(R.size() * 2) == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
    }
}

int TwoStep::nowMs() {
    return static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
}

MP TwoStep::outputSolver(bool projection) {
    if(!outputSolverInit){
        // Output Matching Order Heuristics
        for(int i = cir1.getInputNum() ; i < cir1.getInputNum() + cir1.getOutputNum() ; i++){
            cir1Output.push_back(cir1.fromOrderToName(i));
        }
        for(int i = cir2.getInputNum() ; i < cir2.getInputNum() + cir2.getOutputNum() ; i++){
            cir2Output.push_back(cir2.fromOrderToName(i));
        }
        sort(cir1Output.begin(), cir1Output.end(), [this](const std::string &a, const std::string &b) -> bool {
            return this->heuristicsOrderCmp(a, b);
        });
        for(int i = 0 ; i < static_cast<int>(cir1Output.size()) ; i++){
            cir1OutputMap[cir1Output[i]] = i;
        }
        sort(cir2Output.begin(), cir2Output.end(), [this](const std::string &a, const std::string &b) -> bool {
            return this->heuristicsOrderCmp(a, b);
        });
        for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
            cir2OutputMap[cir2Output[i]] = i;
        }
        // init output match matrix
        initVe.resize(cir2Output.size());
        for(auto & i : initVe){
            i.resize(cir1Output.size() * 2, true);
        }
        if(cir1Output.size() == cir2Output.size()){
            vector<int> groupId = generateOutputGroups(cir1Output, cir2Output);
            for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size()) ; q += 2){
                    if(groupId[i] != groupId[q / 2]){
                        initVe[i][q] = initVe[i][q + 1] = 0;
                    }
                }
            }
        }
        outputSolverInit = true;
        cir1Choose.resize(cir1Output.size(), -1);
        cir1Choose.resize(cir2Output.size(), -1);
        return outputSolver(false);
    }else{
        for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
            if(cir2Choose[i] == -1){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size() * 2) ; q++){
                    if(!(projection && cir1Choose[q / 2] != 0) && initVe[i][q]){
                        cir1Choose[q / 2]++;
                        MP re = MP(cir1Output[q / 2] + (q % 2 == 0 ? "" : "\'"), cir2Output[i]);
                        backtrace.push(re);
                        return re;
                    }
                }
            }
        }
    }
#ifdef DBG
    cout << "[TwoStep] ERROR: can not handle output solver.\n";
            exit(1);
#endif
}


void TwoStep::outputSolverPop() {
    MP last = backtrace.top();
    backtrace.pop();
    cir1Choose[cir1OutputMap[last.first]]--;
    cir2Choose[cir2OutputMap[last.second]] = -1;
}


bool TwoStep::inputSolver() {
    return false;
}

bool TwoStep::heuristicsOrderCmp(string a, string b) {

    std::function<std::set<std::string>(std::string)> funSupport;
    if (a[0] == '!') {
        funSupport = [this](auto && PH1) { return cir1.getFunSupport(std::forward<decltype(PH1)>(PH1)); };
    } else {
        funSupport = [this](auto && PH1) { return cir2.getFunSupport(std::forward<decltype(PH1)>(PH1)); };
    }
    int funSupportSizeA = funSupport(a).size();
    int funSupportSizeB = funSupport(b).size();
    std::function<std::set<std::string>(std::string)> strSupport;
    if (a[0] == '!') {
        strSupport = [=](std::string name) { return cir1.getSupport(name); };
    } else {
        strSupport = [=](std::string name) { return cir2.getSupport(name); };
    }
    int strSupportSizeA = strSupport(a).size();
    int strSupportSizeB = strSupport(b).size();

    if(funSupportSizeA == funSupportSizeB){
        return strSupportSizeA < strSupportSizeB;
    }else{
        return funSupportSizeA < funSupportSizeB;
    }
}

vector<int> TwoStep::generateOutputGroups(vector<string> &f, vector<string> &g) {
    vector<int> re;
    re.resize(f.size());
    vector<vector<int> > group;
    group.emplace_back();
    for(int i = static_cast<int>(f.size()); i >= 0 ; i--){
        group.back().push_back(i);
        if(cir1.getFunSupport(f[i]).size() > cir2.getFunSupport(g[i]).size()){
            group.emplace_back();
        }
    }
    for(int i = 0 ; i < static_cast<int>(group.size()) ; i++){
        for(int q = 0 ; q < static_cast<int>(group[i].size()) ; q++){
            re[group[i][q]] = i;
        }
    }
    return re;
}
