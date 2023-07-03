//
// Created by grorge on 6/25/23.
//
#include <chrono>
#include <utility>
#include "TwoStep.h"
#include "CNF.h"
#include "utility.h"
#include "parser.h"

void TwoStep::start() {
    int startMs = nowMs();
    bool optimal = false, timeout = false, projection = false;
    vector<MP> R;
    while (!optimal && !timeout){
        MP newPair = outputSolver(projection, R);
        if(!newPair.first.empty()){
            R.push_back(newPair);
        }else{
            if(!projection){
                projection = true;
            }else{
                R.pop_back();
                outputSolverPop();
                projection = false;
            }
            continue;
        }
        clauseNum.push_back(clauseStack.size());
        vector<MP> inputMatch = inputSolver(R);
        if(inputMatch.empty()){
            R.pop_back();
            outputSolverPop();
        }else{
            map<string, int> nameMap;
            vector<Group> inputGroups, outputGroups;
            vector<string> one, zero;
            int matchOutput = 0;
            for(auto pair : R){
                if(nameMap.find(pair.first) == nameMap.end()){
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
            for(auto pair : inputMatch){
                if(pair.first.size() == 1){
                    if(pair.first == "1"){
                        one.push_back(pair.second);
                    }else{
                        zero.push_back(pair.second);
                    }
                }else{
                    if(nameMap.find(pair.first) == nameMap.end()){
                        Group newGroup;
                        newGroup.cir1 = pair.first;
                        nameMap[pair.first] = inputGroups.size();
                        inputGroups.push_back(newGroup);
                    }
                    inputGroups[nameMap[pair.first]].cir2.push_back(pair.second);
                    inputGroups[nameMap[pair.first]].invVector.push_back(false);
                }
            }
            parseOutput(outputFilePath, OutputStructure{inputGroups, outputGroups, one, zero}, matchOutput);
            if( matchOutput == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
    }
}

int TwoStep::nowMs() {
    return static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
}

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
            vector<int> groupId = generateOutputGroups(cir1Output, cir2Output);
            for(int i = 0 ; i < static_cast<int>(cir2Output.size()) ; i++){
                for(int q = 0 ; q < static_cast<int>(cir1Output.size()) ; q += 2){
                    if(groupId[i] != groupId[q / 2]){
                        initVe[i][q] = initVe[i][q + 1] = false;
                    }
                }
            }
        }
        cir1Choose.resize(cir1Output.size(), -1);
        cir1Choose.resize(cir2Output.size(), -1);
        lastCir1 = lastCir2 = 0;
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
                    if(!(projection && cir1Choose[q / 2] != 0) && initVe[i][q]){
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
    }
    return {};
}


void TwoStep::outputSolverPop() {
    MP last = backtrace.top();
    backtrace.pop();
    cir1Choose[cir1OutputMap[last.first]]--;
    cir2Choose[cir2OutputMap[last.second]] = -1;
    while (static_cast<int>(clauseStack.size()) > clauseNum.back()){
        clauseStack.pop_back();
    }
    clauseNum.pop_back();
}


vector<MP> TwoStep::inputSolver(vector<MP> &R) {
    set<string> cir1ChoosePort;
    set<string> cir2ChoosePort;
    for(auto pair : R){
        if(pair.first.back() == '\''){
            pair.first.pop_back();
        }
        cir1ChoosePort.insert(pair.first);
        for(const string& funSupportPort: cir1.getFunSupport(pair.first)){
            cir1ChoosePort.insert(funSupportPort);
        }
        if(pair.second.back() == '\''){
            pair.second.pop_back();
        }
        cir2ChoosePort.insert(pair.first);
        for(const string& funSupportPort: cir2.getFunSupport(pair.second)){
            cir2ChoosePort.insert(funSupportPort);
        }
    }
    vector<string> cir1Erase, cir2Erase;
    for(int i = 0 ; i < cir1.getInputNum() + cir1.getOutputNum(); i++){
        if(cir1ChoosePort.find(cir1.fromOrderToName(i)) == cir1ChoosePort.end()){
            cir1Erase.push_back(cir1.fromOrderToName(i));
        }
    }
    for(int i = 0 ; i < cir2.getInputNum() + cir2.getOutputNum(); i++){
        if(cir2ChoosePort.find(cir2.fromOrderToName(i)) == cir2ChoosePort.end()){
            cir2Erase.push_back(cir2.fromOrderToName(i));
        }
    }
    AIG cir1Copy = cir1;
    AIG cir2Copy = cir2;
    cir1Copy.erasePort(cir1Erase);
    cir2Copy.erasePort(cir2Erase);
    CNF mappingSpace;
    int baseLength = (cir1Copy.getInputNum() + 1) * 2;
    mappingSpace.maxIdx = cir2Copy.getInputNum() * baseLength;
    for(int i = 0 ; i < cir2Copy.getInputNum() ; i++){
        for(int q = 0 ; q < cir1Copy.getInputNum(); q ++){
            // CNF base 1
            mappingSpace.varMap[cir1Copy.fromOrderToName(q) + "_" + cir2Copy.fromOrderToName(i)] = i * baseLength + q * 2 + 1;
            mappingSpace.varMap[cir1Copy.fromOrderToName(q) + '\'' + "_" + cir2Copy.fromOrderToName(i)] = i * baseLength + q * 2 + 1 + 1;
        }
    }
    for(int i = 0 ; i < cir2Copy.getInputNum() ; i++){
        mappingSpace.varMap["0_" + cir2Copy.fromOrderToName(i)] = i * baseLength + cir1Copy.getInputNum() * 2 + 1;
        mappingSpace.varMap["1_" + cir2Copy.fromOrderToName(i)] = i * baseLength + cir1Copy.getInputNum() * 2 + 1 + 1;
    }
    for(int i = 0 ; i < cir2Copy.getInputNum() ; i++){
        vector<int> clause;
        clause.reserve(baseLength);
        for(int q = 0 ; q < baseLength ; q++){
            clause.push_back(i * baseLength + q + 1);
        }
        clause.push_back(0);
        mappingSpace.clauses.emplace_back(clause);
        for(int q = 0 ; q < baseLength ; q++){
            for(int k = q + 1 ; k < baseLength ; k++){
                clause.clear();
                clause.push_back((i * baseLength + q + 1) * -1);
                clause.push_back((i * baseLength + k + 1) * -1);
                clause.push_back(0);
                mappingSpace.clauses.emplace_back(clause);
            }
        }
    }
    // recover learning clause
    for(const auto& clauses : clauseStack){
        vector<int> clause;
        clause.reserve(clauses.size());
        for(const auto &key : clauses){
#ifdef DBG
            if(mappingSpace.varMap.find(key) == mappingSpace.varMap.end()){
                cout << "[TwoStep] Error: cant not recover " << key << endl;
                exit(0);
            }
#endif
            clause.emplace_back(mappingSpace.varMap[key]);
        }
        clause.push_back(0);
        mappingSpace.clauses.push_back(clause);
    }
    vector<MP> mapping;
    while (true){
        mapping = solveMapping(mappingSpace, cir1Copy, cir2Copy, baseLength);
        pair<vector<bool>, vector<bool> > counter = solveMiter(mapping, R, cir1Copy, cir2Copy);
        if(counter.first.empty()){
            break;
        }
        reduceSpace(mappingSpace, counter, baseLength, cir1Copy, cir2Copy, mapping);
    }

    return mapping;
}

bool TwoStep::heuristicsOrderCmp(const string& a, const string& b) {

    std::function<std::set<std::string>(std::string)> funSupport;
    if (a[0] == '!') {
        funSupport = [this](auto && PH1) { return cir1.getFunSupport(std::forward<decltype(PH1)>(PH1)); };
    } else {
        funSupport = [this](auto && PH1) { return cir2.getFunSupport(std::forward<decltype(PH1)>(PH1)); };
    }
    int funSupportSizeA = static_cast<int>(funSupport(a).size());
    int funSupportSizeB = static_cast<int>(funSupport(b).size());
    std::function<std::set<std::string>(std::string)> strSupport;
    if (a[0] == '!') {
        strSupport = [=](std::string name) { return cir1.getSupport(std::move(name)); };
    } else {
        strSupport = [=](std::string name) { return cir2.getSupport(std::move(name)); };
    }
    int strSupportSizeA = static_cast<int>(strSupport(a).size());
    int strSupportSizeB = static_cast<int>(strSupport(b).size());

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

vector<MP> TwoStep::solveMapping(CNF &mapping, AIG &cir1, AIG &cir2, const int baseLength) {
    vector<MP> re;
    ofstream of;
    string fileName = "TwoStepSolveMapping.cnf";
    of.open(fileName);
    of << mapping.getRow();
    of.close();
    solverResult result = SAT_solver(fileName.c_str());
    if(result.satisfiable){
#ifdef DBG
        if(result.inputSize != cir2.getInputNum() * baseLength){
            cout << "[TwoStep] Error: sat solver return non expected input size." << endl;
            exit(1);
        }
#endif
        for(int i = 0 ; i < cir2.getInputNum() ; i++){
            for(int q = 0 ; q < (cir1.getInputNum() + 1) * 2 ; q++){
                if(result.input[i * baseLength + q]){
                    if(q >= cir1.getInputNum() * 2){
                        if(q == cir1.getInputNum() * 2 ){
                            re.emplace_back( "0",cir2.fromOrderToName(i));
                        }else{
                            re.emplace_back( "1",cir2.fromOrderToName(i));
                        }
                    }else{
                        re.emplace_back(MP(cir1.fromOrderToName(q / 2) + (q % 2 == 0 ? "" : "\'"), cir2.fromOrderToName(i)));
                    }
                }
            }
        }
    }
    return re;
}

pair<vector<bool>, vector<bool>>
TwoStep::solveMiter(const vector<MP> &inputMatchPair, const vector<MP> &outputMatchPair, AIG &cir1, AIG &cir2) {
    set<string> projectiveInput, projectiveOutput;
    vector<string> inverter;
    for(const auto &pair : outputMatchPair){
        auto [gateName, negative] = analysisName(pair.first);
        if(negative)cir2.invertGate(pair.second);
        if(projectiveOutput.find(pair.first) == projectiveOutput.end()){
            cir2.changeName(pair.second, cir1.cirName + gateName);
            projectiveOutput.insert(cir1.cirName + pair.first);
        }else{
            cir1.copyOutput(cir1.cirName + pair.first, cir2.cirName + pair.second);
        }
    }
    for(const auto &pair : inputMatchPair){
        if(pair.first.size() == 1){
            cir2.setConstant(pair.second, stoi(pair.first));
        }else{
            auto [gateName, negative] = analysisName(pair.first);
            if(projectiveInput.find(pair.first) == projectiveInput.end()){
                cir2.changeName(pair.second, cir1.cirName + gateName);
                inverter.push_back(pair.second);
                projectiveInput.insert(cir1.cirName + pair.first);
            }else{
                cir2.exportInput(pair.second, cir1.cirName + pair.first, negative);
            }
        }
    }
    for(const auto& port : inverter){
        cir2.invertGate(port);
    }
    CNF miter;
    solverResult result = ::solveMiter(cir1, cir2, &miter);
    if(result.satisfiable){
        vector<bool> cir1Input, cir2Input;
        auto getCounter = [&](vector<bool> &cirInput, AIG &cir){
            cirInput.resize(cir.getInputNum());
            for(int idx = 0 ; idx < cir.getInputNum() ; idx++){
                string name = cir.fromOrderToName(idx);
                //TODO check base and if input need invert
                cirInput[idx] = result.input[miter.varMap[name] - 1];
            }
        };
        getCounter(cir1Input, cir1);
        getCounter(cir2Input, cir2);
        return {cir1Input, cir2Input};
    }else{
        return {};
    }
}

pair<string, bool> TwoStep::analysisName(string name) {
    bool negative = false;
    if(name.back() == '\''){
        negative = true;
        name.pop_back();
    }
    name.erase(name.front());
    return {name, negative};
}
void
TwoStep::reduceSpace(CNF &mappingSpace, const pair<vector<bool>, vector<bool>> &counter, const int baseLength,
                     AIG &cir1, AIG &cir2, const vector<MP> &mapping) {
    auto& [cir1Input, cir2Input] = counter;
    vector<int> cir1NonRedundant = getNonRedundant(cir1Input, cir1), cir2NonRedundant = getNonRedundant(cir2Input, cir2);
    vector<int> clause;
    vector<string> record;
    for(auto i : cir2NonRedundant){
        for(auto j : cir1NonRedundant){
            if(cir1Input[j] == cir2Input[i]){
                clause.push_back(i * baseLength + j * 2 + 1 + 1);
                record.push_back(cir1.fromOrderToName(j) + '\'' + "_" + cir2.fromOrderToName(i));
            }else{
                clause.push_back(i * baseLength + j * 2 + 1);
                record.push_back(cir1.fromOrderToName(j) + "_" + cir2.fromOrderToName(i));
            }
        }
    }
    clauseStack.push_back(record);
    clause.push_back(0);
    mappingSpace.clauses.push_back(clause);
    clause.clear();
    record.clear();
    //TODO record constant input to replace search
    for(auto pair : mapping){
        if(pair.first.size() == 1){
            if(pair.first == "1"){
                clause.push_back(cir2.fromNameToOrder(pair.second) * baseLength  + cir1.getInputNum() + 1);
                record.push_back("0_" + pair.second);
            }else{
                clause.push_back(cir2.fromNameToOrder(pair.second) * baseLength  + cir1.getInputNum() + 1 + 1);
                record.push_back("1_" + pair.second);
            }
        }
    }
    clauseStack.push_back(record);
    clause.push_back(0);
    mappingSpace.clauses.push_back(clause);

}


vector<int> TwoStep::getNonRedundant(const vector<bool> &input, AIG &cir) {
    vector<int> re;
    for(int i = 0 ; i < cir.getInputNum() ; i++){
        re.push_back(i);
        vector<bool> input2 = input;
        input2[i] = !input2[i];
        // TODO check need fix output that is different or just have different output
        if(cir.generateOutput(input) == cir.generateOutput(input2)){
            re.pop_back();
        }
    }
    return re;
}
