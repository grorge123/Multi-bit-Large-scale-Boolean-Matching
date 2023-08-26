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

        if(enableOutputBus){
            for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size() * 2) ; q += 2){
                    bool cir1InBus = (cir1BusMapping.find(cir1Output[q/2]) != cir1BusMapping.end());
                    bool cir2InBus = (cir2BusMapping.find(cir2Output[i]) != cir2BusMapping.end());
                    if(cir1InBus && cir2InBus){
                        if(cir1OutputBus[cir1BusMapping[cir1Output[q/2]]].size() != cir2OutputBus[cir2BusMapping[cir2Output[i]]].size()){
                            initVe[i][q] = initVe[i][q + 1] = false;
                        }
                    }else{
                        if(cir1InBus ^ cir2InBus){
                            initVe[i][q] = initVe[i][q + 1] = false;
                        }
                    }
                }
            }
        }
        // with only negative output match could solve case 5 but it is has bug on case 10
//        for(int q = 0 ; q < static_cast<int>(cir1Output.size()) ; q++) {
//            for (int i = 0; i < static_cast<int>(cir2Output.size()); i++) {
//                if(cir1OutputMaxSup[cir1.fromNameToOrder(cir1Output[q]) - cir1.getInputNum()] < cir2OutputMaxSup[cir2.fromNameToOrder(cir2Output[i]) - cir2.getInputNum()]){
//                    initVe[i][q * 2] = initVe[i][q * 2 + 1] = false;
//                }
//            }
//        }

        cir1Choose.resize(cir1Output.size(), 0);
        cir2Choose.resize(cir2Output.size(), -1);
//        lastCir1 = lastCir2 = 0;
        outputSolverInit = true;
        clauseStack.clear();
        clauseNum.clear();

        // TODO delete this

//        cout << "cir1Output:" << endl;
//        for(int i = 0 ; i < static_cast<int>(cir1Output.size()) ; i++){
//            vector<bool> inp(cir1.getInputNum());
//            vector<bool> inp2(cir1.getInputNum());
//            inp2[cir1.fromNameToOrder(*cir1.getSupport(cir1Output[i], 1).begin())] = 1;
//            vector<bool> oup = cir1.generateOutput(inp);
//            vector<bool> oup2 = cir1.generateOutput(inp2);
//            bool eq = oup[cir1.fromNameToOrder(cir1Output[i]) - cir1.getInputNum()] == 0;
//            cout << cir1Output[i] << " " << hGroupId[i] << " " << cir1.getSupport(cir1Output[i], 1).size() << " " << *cir1.getSupport(cir1Output[i], 1).begin() << " " << (eq ? "buf" : "not") << endl;
////            if(hGroupId[i] != hGroupId[0])break;
//        }
//        cout << "cir2Output:" << endl;
//        for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
//            vector<bool> inp(cir2.getInputNum());
//            vector<bool> inp2(cir2.getInputNum());
//            inp2[cir2.fromNameToOrder(*cir2.getSupport(cir2Output[i], 1).begin())] = 1;
//            vector<bool> oup = cir2.generateOutput(inp);
//            vector<bool> oup2 = cir2.generateOutput(inp2);
//            bool eq = oup[cir2.fromNameToOrder(cir2Output[i]) - cir2.getInputNum()] == 0;
//            cout << cir2Output[i] << " " << hGroupId[i] << " " << cir2.getSupport(cir2Output[i], 1).size() << " " << *cir2.getSupport(cir2Output[i], 1).begin() << " " << (eq ? "buf" : "not") << endl;
////            if(hGroupId[i] != hGroupId[0])break;
//        }

//        int possible = 1;
//        for (int i = 0; i < static_cast<int>(cir2Output.size()); i++) {
////            if(hGroupId[i] != hGroupId[0])break;
//            int cntPos = 0;
//            for(int q = 0 ; q < static_cast<int>(cir1Output.size()) ; q++) {
////                if(hGroupId[q] != hGroupId[0])break;
//                if(initVe[i][q * 2])cntPos++;
//                if(initVe[i][q * 2 + 1])cntPos++;
//                cout << initVe[i][q * 2] << " " << initVe[i][q * 2 + 1] << " ";
//            }
//            possible *= cntPos;
//            cout << cntPos << endl;
//        }
//        cout <<"possible:"<< possible << endl;

//        exit(0);


        return outputSolver(false, R);
    }else{
        auto busConflict = [&](const MP& mp) -> bool{
            bool conflict = false;
            if(cir1BusMapping.find(mp.first) == cir1BusMapping.end() || cir2BusMapping.find(mp.second) == cir2BusMapping.end())return conflict;
            if(cir1OutputBusMatch.find(cir1BusMapping[mp.first]) != cir1OutputBusMatch.end()){
                if(cir1OutputBusMatch[cir1BusMapping[mp.first]] != cir2BusMapping[mp.second]) conflict = true;
            }
            if(cir2OutputBusMatch.find(cir2BusMapping[mp.second]) != cir2OutputBusMatch.end()){
                if(cir2OutputBusMatch[cir2BusMapping[mp.second]] != cir1BusMapping[mp.first]) conflict = true;
            }
            return conflict;
        };
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
                        if(enableOutputBus){
                            if(busConflict(re))continue;
                        }
                        nowSelect.insert(re);
                        size_t hashValue = hashSet(nowSelect);
                        if(forbid.find(hashValue) != forbid.end()){
                            nowSelect.erase(re);
                            continue;
                        }else{
                            forbid.insert(hashValue);
                        }
                        if(cir1BusMapping.find(re.first) != cir1BusMapping.end() && cir2BusMapping.find(re.second) != cir2BusMapping.end()){
                            cir1OutputBusMatch[cir1BusMapping[re.first]] = cir2BusMapping[re.second];
                            cir2OutputBusMatch[cir2BusMapping[re.second]] = cir1BusMapping[re.first];
                        }
                        cir1Choose[q / 2]++;
                        cir2Choose[i] = q;
                        backtrace.push(re);
                        return re;
                    }
                }
//                if(cir2Choose[i] == -1)return {};
            }
        }
        return {};
    }
}
int cnt = 0;

void TwoStep::outputSolverPop(vector<MP> &R) {
    MP last = backtrace.top();
    R.pop_back();
    backtrace.pop();
    if(last.first.back() == '\'')last.first.pop_back();
    bool eraseBusMatch = true;
    for(const auto &pair : R){
        if(cir1BusMapping.find(pair.first) != cir1BusMapping.end() && cir2BusMapping.find(pair.second) != cir2BusMapping.end()){
            if(cir1BusMapping[pair.first] == cir1BusMapping[last.first])eraseBusMatch = false;
        }
    }
    if(eraseBusMatch){
        cir1OutputBusMatch.erase(cir1BusMapping[last.first]);
        cir2OutputBusMatch.erase(cir2BusMapping[last.second]);
    }
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

    if(funSupportSizeA != funSupportSizeB) {
        return funSupportSizeA < funSupportSizeB;
    }else if(strSupportSizeA != strSupportSizeB){
        return strSupportSizeA < strSupportSizeB;
    }else{
        return busSupportA.size() < busSupportB.size();
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