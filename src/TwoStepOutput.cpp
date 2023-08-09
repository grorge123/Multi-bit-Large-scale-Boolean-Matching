//
// Created by grorge on 7/22/23.
//

#include <utility>
#include "TwoStep.h"
#include "utility.h"
#include "parser.h"

MP TwoStep::outputSolver(bool projection, vector<MP> &R) {
    if(!outputSolverInit){
        forbid.clear();
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
            hGroupId = generateOutputGroups(cir1Output, cir2Output);
            for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size() * 2) ; q += 2){
                    if(hGroupId[i] != hGroupId[q / 2]){
                        initVe[i][q] = initVe[i][q + 1] = false;
                    }
                }
            }
        }
        cir1Choose.resize(cir1Output.size(), 0);
        cir2Choose.resize(cir2Output.size(), -1);
//        lastCir1 = lastCir2 = 0;
        outputSolverInit = true;
        clauseStack.clear();
        clauseNum.clear();
        return outputSolver(false, R);
    }else{
        set<MP> nowSelect;
        for(const auto& mp : R){
            nowSelect.insert(mp);
        }
        // TODO optimize record last choose number
        for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
            if(cir2Choose[i] == -1){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size() * 2) ; q++){
                    if(!projection &&(cir1Choose[q / 2] != 0))continue;
                    if(initVe[i][q]){
                        MP re = MP(cir1Output[q / 2] + (q % 2 == 0 ? "" : "\'"), cir2Output[i]);
                        if(nowSelect.find(re) != nowSelect.end())continue;
                        nowSelect.insert(re);
                        size_t hashValue = hashSet(nowSelect);
                        if(forbid.find(hashValue) != forbid.end()){
                            nowSelect.erase(re);
                            continue;
                        }else{
                            forbid.insert(hashValue);
                        }
                        cir1Choose[q / 2]++;
                        cir2Choose[i] = q;
                        backtrace.push(re);
                        return re;
                    }
                }
            }
        }
        return {};
    }
}
int cnt = 0;

void TwoStep::outputSolverPop() {
    MP last = backtrace.top();
    backtrace.pop();
    if(last.first.back() == '\'')last.first.pop_back();
    cir1Choose[cir1OutputMap[last.first]]--;
    cir2Choose[cir2OutputMap[last.second]] = -1;
    while (static_cast<int>(clauseStack.size()) > clauseNum.back()){
        clauseStack.pop_back();
    }
    clauseNum.pop_back();

}

bool TwoStep::heuristicsOrderCmp(const string& a, const string& b) {

    std::function<std::set<std::string>(std::string)> funSupport;
    if (a[0] == '!') {
        funSupport = [this](auto && PH1) { return cir1.getSupport(std::forward<decltype(PH1)>(PH1), 1); };
    } else {
        funSupport = [this](auto && PH1) { return cir2.getSupport(std::forward<decltype(PH1)>(PH1), 1); };
    }
    int funSupportSizeA = static_cast<int>(funSupport(a).size());
    int funSupportSizeB = static_cast<int>(funSupport(b).size());
    std::function<std::set<std::string>(std::string)> strSupport;
    if (a[0] == '!') {
        strSupport = [=](std::string name) { return cir1.getSupport(std::move(name), 2); };
    } else {
        strSupport = [=](std::string name) { return cir2.getSupport(std::move(name), 2); };
    }
    int strSupportSizeA = static_cast<int>(strSupport(a).size());
    int strSupportSizeB = static_cast<int>(strSupport(b).size());

    set<int> busSupportA;
    set<int> busSupportB;
    if(a[0] == '!'){
        for(const auto& port : funSupport(a)){
            if(cir1BusMapping.find(port) != cir1BusMapping.end()){
                busSupportA.insert(cir1BusMapping[port]);
            }
        }
        for(const auto& port : funSupport(b)){
            if(cir1BusMapping.find(port) != cir1BusMapping.end()){
                busSupportB.insert(cir1BusMapping[port]);
            }
        }
    }else{
        for(const auto& port : funSupport(a)){
            if(cir2BusMapping.find(port) != cir2BusMapping.end()){
                busSupportA.insert(cir2BusMapping[port]);
            }
        }
        for(const auto& port : funSupport(b)){
            if(cir2BusMapping.find(port) != cir2BusMapping.end()){
                busSupportB.insert(cir2BusMapping[port]);
            }
        }
    }

    if(funSupportSizeA == funSupportSizeB){
        if(strSupportSizeA == strSupportSizeB){
            return busSupportA.size() < busSupportB.size();
        }else{
            return strSupportSizeA < strSupportSizeB;
        }
    }else{
        return funSupportSizeA < funSupportSizeB;
    }
}

vector<int> TwoStep::generateOutputGroups(vector<string> &f, vector<string> &g) {
    vector<int> re;
    re.resize(f.size());
    vector<vector<int> > group;
    group.emplace_back();
    for(int i = static_cast<int>(f.size()) - 1; i >= 0 ; i--){
        group.back().push_back(i);
        if(i > 0 && cir1.getSupport(f[i], 1).size() > cir2.getSupport(g[i - 1], 1).size()){
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