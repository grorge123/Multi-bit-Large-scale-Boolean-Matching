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
    startMs = nowMs();
    bool optimal = false, timeout = false, projection = false;
    vector<MP> R;
    while (!optimal && !timeout){
        auto [newPair, hashValue] = outputSolver(projection, R);
        backtraceHash.push(hashValue);
        if(!newPair.first.empty()){
            R.push_back(newPair);
        }else{
            if(!projection){
                projection = true;
            }else{
                if(R.empty()){
                    optimal = true;
                    break;
                }
                forbid.insert(backtraceHash.top());
                R.pop_back();
                outputSolverPop();
                projection = false;
            }
            continue;
        }
        if(verbose){
            cout << "Output Pair:" << projection << " " << R.size() << endl;
            for(const auto& pair : R){
                cout << pair.first << ' ' << pair.second << endl;
            }
            recordMs();
        }
        clauseNum.push_back(clauseStack.size());
        vector<MP> inputMatch = inputSolver(R, true);
        if (inputMatch.empty()) {
            R.pop_back();
            outputSolverPop();
            while (true) {
                inputMatch = inputSolver(R, false);
                if(inputMatch.empty()){
                    forbid.insert(backtraceHash.top());
                    R.pop_back();
                    outputSolverPop();
                    break;
                }
            }
        } else {
            int matchOutput = recordOutput(inputMatch, R);
            if (matchOutput == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
        iteratorCounter++;
    }
    cout << "Final iteration: " << iteratorCounter << endl;
}

int TwoStep::nowMs() {
    return static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
}
int recordCounter = 0;
void TwoStep::recordMs() {
    if(!verbose)return;
    int now = nowMs();
    cout <<"During:"<< now - lastTime << " Iteration:" << recordCounter << endl;
    recordCounter++;
    lastTime = now;
}

pair<MP, size_t> TwoStep::outputSolver(bool projection, vector<MP> &R) {
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
        cir1Choose.resize(cir1Output.size(), 0);
        cir2Choose.resize(cir2Output.size(), -1);
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
                    if(!(!projection && cir1Choose[q / 2] != 0) && initVe[i][q]){
                        MP re = MP(cir1Output[q / 2] + (q % 2 == 0 ? "" : "\'"), cir2Output[i]);
                        if(nowSelect.find(re) != nowSelect.end())continue;
                        nowSelect.insert(re);
                        size_t hashValue = hashSet(nowSelect);
                        if(forbid.find(hashValue) != forbid.end()){
                            nowSelect.erase(re);
                            continue;
                        }
//                        else{
//                            forbid.insert(hashValue);
//                        }
                        cir1Choose[q / 2]++;
                        cir2Choose[i] = q;
                        backtrace.push(re);
                        return {re, hashValue};
                    }
                }
            }
        }
        return {MP{}, hashSet(nowSelect)};
    }
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
    backtraceHash.pop();
    inputStack.pop();
}

vector<MP> TwoStep::inputSolver(vector<MP> &R, bool init) {
    //TODO define input solver stack recore input solver information
    if(init) {
        set<string> cir1ChoosePort;
        set<string> cir2ChoosePort;
        for (auto pair: R) {
            if (pair.first.back() == '\'') {
                pair.first.pop_back();
            }
            cir1ChoosePort.insert(pair.first);
            if (pair.second.back() == '\'') {
                pair.second.pop_back();
            }
            cir2ChoosePort.insert(pair.second);
        }
        vector<string> cir1Erase, cir2Erase;
        vector<string> cir1Constant, cir2Constant;
        auto eraseNonUsedPort = [](AIG &cir, set<string> &cirChoosePort,
                                   vector<string> &cirConstant) -> vector<string> {
            set<string> cirErase;
            for (int i = cir.getInputNum(); i < cir.getInputNum() + cir.getOutputNum(); i++) {
                if (cirChoosePort.find(cir.fromOrderToName(i)) == cirChoosePort.end()) {
                    cirErase.insert(cir.fromOrderToName(i));
                }
            }
            for (int order = 0; order < cir.getInputNum(); order++) {
                int eraseFlag = 0;
                for (const string &funSupportPort: cir.getSupport(cir.fromOrderToName(order), 1)) {
                    if (cirErase.find(funSupportPort) == cirErase.end()) {
                        eraseFlag = 1;
                        break;
                    }
                }
                if (eraseFlag == 0) {
                    for (const string &strSupportPort: cir.getSupport(cir.fromOrderToName(order), 2)) {
                        if (cirErase.find(strSupportPort) == cirErase.end()) {
                            eraseFlag = 2;
                            break;
                        }
                    }
                }
                if (eraseFlag == 0) {
                    cirErase.insert(cir.fromOrderToName(order));
                } else if (eraseFlag == 2) {
                    cirConstant.push_back(cir.fromOrderToName(order));
                }
            }
            vector<string> re;
            re.reserve(cirErase.size());
            for (const auto &erase: cirErase) {
                re.push_back(erase);
            }
            return re;
        };
        cir1Erase = eraseNonUsedPort(cir1, cir1ChoosePort, cir1Constant);
        cir2Erase = eraseNonUsedPort(cir2, cir2ChoosePort, cir1Constant);
        AIG cir1Reduce = cir1;
        AIG cir2Reduce = cir2;
        for (const auto &name: cir1Constant) {
            cir1.setConstant(name, 0);
        }
        for (const auto &name: cir2Constant) {
            cir2.setConstant(name, 0);
        }
        cir1Reduce.erasePort(cir1Erase);
        cir2Reduce.erasePort(cir2Erase);
        CNF mappingSpace;
        int baseLength = (cir1Reduce.getInputNum() + 1) * 2;
        mappingSpace.maxIdx = cir2Reduce.getInputNum() * baseLength;
        for (int i = 0; i < cir2Reduce.getInputNum(); i++) {
            for (int q = 0; q < cir1Reduce.getInputNum(); q++) {
                // CNF base 1
                mappingSpace.varMap[cir1Reduce.fromOrderToName(q) + "_" + cir2Reduce.fromOrderToName(i)] =
                        i * baseLength + q * 2 + 1;
                mappingSpace.varMap[cir1Reduce.fromOrderToName(q) + '\'' + "_" + cir2Reduce.fromOrderToName(i)] =
                        i * baseLength + q * 2 + 1 + 1;
            }
        }
        for (int i = 0; i < cir2Reduce.getInputNum(); i++) {
            mappingSpace.varMap["0_" + cir2Reduce.fromOrderToName(i)] =
                    i * baseLength + cir1Reduce.getInputNum() * 2 + 1;
            mappingSpace.varMap["1_" + cir2Reduce.fromOrderToName(i)] =
                    i * baseLength + cir1Reduce.getInputNum() * 2 + 1 + 1;
        }
        //Equation 3
        for (int i = 0; i < cir2Reduce.getInputNum(); i++) {
            vector<int> clause;
            clause.reserve(baseLength);
            for (int q = 0; q < baseLength; q++) {
                clause.push_back(i * baseLength + q + 1);
            }
            mappingSpace.addClause(clause);
            clause.clear();
            for (int q = 0; q < baseLength; q++) {
                for (int k = q + 1; k < baseLength; k++) {
                    clause.push_back((i * baseLength + q + 1) * -1);
                    clause.push_back((i * baseLength + k + 1) * -1);
                    mappingSpace.addClause(clause);
                    clause.clear();
                }
            }
        }
        if (cir1Reduce.getInputNum() == cir2Reduce.getInputNum()) {
            //disable match constant
            for (int i = 0; i < cir2Reduce.getInputNum(); i++) {
                vector<int> clause;
                clause.push_back(-1 * (i * baseLength + cir1Reduce.getInputNum() * 2 + 1));
                mappingSpace.addClause(clause);
                clause.clear();
                clause.push_back(-1 * (i * baseLength + cir1Reduce.getInputNum() * 2 + 1 + 1));
                mappingSpace.addClause(clause);
            }
            // disable input projection
            for (int i = 0; i < cir1Reduce.getInputNum() * 2; i++) {
                vector<int> clause;
                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                    for (int k = q + 1; k < cir2Reduce.getInputNum(); k++) {
                        clause.push_back((q * baseLength + i + 1) * -1);
                        clause.push_back((k * baseLength + i + 1) * -1);
                        mappingSpace.addClause(clause);
                        clause.clear();
                    }
                }
            }
            for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
                vector<int> clause;
                clause.reserve(2 * cir2Reduce.getInputNum());
                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                    clause.push_back(q * baseLength + 2 * i + 1);
                    clause.push_back(q * baseLength + 2 * i + 1 + 1);
                }
                mappingSpace.addClause(clause);
                clause.clear();
            }
        }
        //remove funSupport not equal
        LargeScale inputLg = LargeScale(cir1Reduce, cir2Reduce);
        auto eigenValue = inputLg.calculateEigenvalue();
        for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
            for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                if (eigenValue[cir1Reduce.fromOrderToName(i)] == eigenValue[cir2Reduce.fromOrderToName(q)])continue;
                vector<int> clause;
                clause.push_back(-1 * (q * baseLength + 2 * i + 1));
                mappingSpace.addClause(clause);
                clause.clear();
                clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
                mappingSpace.addClause(clause);
            }
        }
        // recover learning clause
        for (const auto &clauses: clauseStack) {
            vector<int> clause;
            clause.reserve(clauses.size());
            for (const auto &key: clauses) {
#ifdef DBG
                if (mappingSpace.varMap.find(key) == mappingSpace.varMap.end()) {
                    cout << "[TwoStep] Error: cant not recover " << key << endl;
                    exit(1);
                }
#endif
                clause.emplace_back(mappingSpace.varMap[key]);
            }
            mappingSpace.addClause(clause);
            clause.clear();
        }
        inputStack.push({cir1Reduce, cir2Reduce, mappingSpace});
        tsDebug("Reduce Network", cir1Reduce, cir2Reduce);
        return inputSolver(R, false);
    }else{
        auto &[cir1Reduce, cir2Reduce, mappingSpace] = inputStack.top();
        int baseLength = (cir1Reduce.getInputNum() + 1) * 2;
        vector<MP> mapping;
        while (true){
            if(nowMs() - startMs > maxRunTime){
                return {};
            }
            mapping = solveMapping(mappingSpace, cir1Reduce, cir2Reduce, baseLength);
            if(mapping.empty())break;
            // TODO delete this
            if(verbose){
                cout << "Find Mapping" << endl;
                for(const auto& pair : mapping){
                    cout << pair.first << " " << pair.second << endl;
                }
                recordMs();
            }
            pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool> > >, vector<bool> > counter = solveMiter(mapping, R, cir1Reduce, cir2Reduce);
            if(counter.second.empty()){
                break;
            }
            if(verbose){
                cout << "It is counterexample size:" << counter.second.size() << endl;
                recordMs();
            }
            reduceSpace(mappingSpace, counter.second, baseLength, cir1Reduce, cir2Reduce, mapping, counter.first, R);
        }
        return mapping;
    }
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
    for(int i = static_cast<int>(f.size()) - 1; i >= 0 ; i--){
        group.back().push_back(i);
        if(cir1.getSupport(f[i], 1).size() > cir2.getSupport(g[i], 1).size()){
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

vector<MP> TwoStep::solveMapping(CNF &mappingSpace, AIG &cir1, AIG &cir2, const int baseLength) {
    vector<MP> re;
//    ofstream of;
//    string fileName = mappingSpaceFileName;
//    if(mappingSpaceLast != 0){
//        std::streampos firstLineEnd;
//        std::fstream file;
//        file.open(fileName, std::ios::in);
//        std::string firstLine;
//        std::getline(file, firstLine);
//        firstLineEnd = file.tellg();
//        file.close();
//        file.open(fileName, std::ios::out | std::ios::in);
//        string newLine = "p cnf " + to_string(mappingSpace.maxIdx) + " " + to_string(mappingSpace.clauses.size());
//        if (newLine.length() <= firstLine.length()) {
//            file << newLine;
//            for (size_t i = newLine.length(); i < firstLine.length(); ++i) {
//                file.put(' ');
//            }
//        } else {
//#ifdef DBG
//            cout << "[TwoStep] Error: cnf saveSpace not enough !" << endl;
//            exit(1);
//#endif
//        }
//        file.close();
//        of.open(fileName, ios::app);
//        string content;
//        for(int i = mappingSpaceLast ; i < static_cast<int>(mappingSpace.getClauses().size()) ; i++){
//            for(int q : mappingSpace.clauses[i]){
//                content += to_string(q) + " ";
//            }
//            content += "0\n";
//        }
//        of << content;
//        of.close();
//    }else{
//        of.open(fileName);
//        of << mappingSpace.getRaw();
//        of.close();
//    }
//    mappingSpaceLast = static_cast<int>(mappingSpace.getClauses().size());
//    if(verbose){
//        cout << "start solver" << endl;
//    }
//    solverResult result = SAT_solver(fileName.c_str());
//    if(verbose){
//        cout << "end solver" << endl;
//    }
    mappingSpace.solve();
    if(mappingSpace.satisfiable){
#ifdef DBG
        if(static_cast<int>(mappingSpace.satisfiedInput.size()) != cir2.getInputNum() * baseLength){
            cout << "[TwoStep] Error: sat solver return non expected input size. " << mappingSpace.satisfiedInput.size() << " " << cir2.getInputNum() * baseLength << endl;
            exit(1);
        }
#endif
        for(int i = 0 ; i < cir2.getInputNum() ; i++){
            for(int q = 0 ; q < (cir1.getInputNum() + 1) * 2 ; q++){
                if(mappingSpace.satisfiedInput[i * baseLength + q] > 0){
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
pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>>, vector<bool>>
TwoStep::solveMiter(const vector<MP> &inputMatchPair, const vector<MP> &outputMatchPair, AIG cir1, AIG cir2) {
    map<string, pair<int, bool> > cir1NameToOrder; // only input
    map<string, pair<int, bool> > cir2NameToOrder;
    set<string> projectiveInput, projectiveOutput;
    vector<string> inverter2;
    for(const auto &pair : outputMatchPair){
        auto [gateName, negative] = analysisName(pair.first);
        if(projectiveOutput.find(gateName) == projectiveOutput.end()){
            if(negative)cir2.invertGate(pair.second);
            cir2.changeName(pair.second, cir1.cirName + gateName);
            projectiveOutput.insert(gateName);
        }else{
            cir1.copyOutput(cir1.cirName + gateName, pair.second, negative);
        }
    }
    for(const auto &pair : inputMatchPair){
        if(pair.first.size() == 1){
            cir2.setConstant(pair.second, stoi(pair.first));
            cir2NameToOrder[pair.second] = {-1, stoi(pair.first)}; // constant to special order
        }else{
            auto [gateName, negative] = analysisName(pair.first);
            if(projectiveInput.find(gateName) == projectiveInput.end() && projectiveInput.find(gateName + "'") == projectiveInput.end()){
                cir1NameToOrder[cir1.cirName + gateName] = {cir1.fromNameToOrder(cir1.cirName + gateName), false};
                cir2NameToOrder[pair.second] = {cir2.fromNameToOrder(pair.second), negative};
                cir2.changeName(pair.second, cir1.cirName + gateName);
                if(negative)cir2.invertGate(cir1.cirName + gateName);
                projectiveInput.insert(gateName + (negative ? "'" : ""));
            }else{
                cir2NameToOrder[pair.second] = {cir2.fromNameToOrder(cir1.cirName + gateName), negative};
                cir2.exportInput(cir1.cirName + gateName, pair.second, negative);
            }
        }
    }
    vector<string> additionalInput;
    for(int i = 0 ; i < cir1.getInputNum() + cir1.getOutputNum() ; i++){
        string name = cir1.fromOrderToName(i);
        if(!cir2.portExist(name)){
            additionalInput.push_back(name);
            cir1NameToOrder[name] = {cir1.fromNameToOrder(name), false};
        }
    }
    cir2.addFloatInput(additionalInput);
    tsDebug("Matching Network", cir1, cir2);
    //TODO solve miter may mapping may be negative with AIG
    CNF miter;
    AIG miterAIG;
    ::solveMiter(cir1, cir2, miter, miterAIG);
    if(miter.satisfiable){
        vector<bool> counter;
        counter.resize(miterAIG.getInputNum());
        vector<bool> testCir1Input(cir1.getInputNum(), false), testCir2Input(cir2.getInputNum(), false);
        vector<bool> testMiterInput(miterAIG.getInputNum(), false);
        for(int order = 0 ; order < miterAIG.getInputNum() ; order++){
            string name = miterAIG.fromOrderToName(order);
            if(miter.isDC(name)){
                counter[order] = 0;
            }else{
#ifdef DBG
                if(miter.varMap.find(name) == miter.varMap.end()){
                    cout << "[TwoStep] Error: Cant not found " << miterAIG.cirName << "(" << name << ") in cnf." << endl;
                    exit(1);
                }
#endif
                counter[order] = miter.satisfiedInput[miter.varMap[name] - 1];
#ifdef DBG
                testCir1Input[cir1.fromNameToOrder(name)] = counter[order];
                testCir2Input[cir2.fromNameToOrder(name)] = counter[order];
                testMiterInput[order] = counter[order];
                if(verbose){
                    cout << "miter name :" << name << " " << counter[order] << endl;
                }
#endif
            }
        }
#ifdef DBG
        bool notEqual = false;
        vector<bool> testCir1Output = cir1.generateOutput(testCir1Input);
        vector<bool> testCir2Output = cir2.generateOutput(testCir2Input);
        for(int i = cir1.getInputNum() ; i < cir1.getInputNum() + cir1.getOutputNum() ; i++){
            string name = cir1.fromOrderToName(i);
            if(testCir1Output[cir1.fromNameToOrder(name) - cir1.getInputNum()] != testCir2Output[cir2.fromNameToOrder(name) - cir2.getInputNum()]){
                notEqual = true;
            }
        }
        if(verbose){
            cout << "testCir1Input:" << endl;
            for(int i = 0 ; i < static_cast<int>(testCir1Input.size()) ; i++){
                cout << testCir1Input[i] << " ";
            }
            cout << endl;
            cout << "testCir2Input:" << endl;
            for(int i = 0 ; i < static_cast<int>(testCir2Input.size()) ; i++){
                cout << testCir2Input[i] << " ";
            }
            cout << endl;
        }
        if(!notEqual){
            cout << "[TwoStep] SelfTest fail: match network are equal!" <<endl;
            cout <<"miter Result:"<< miterAIG.generateOutput(testMiterInput)[0];
            exit(1);
        }
#endif
        for(auto &order : cir1NameToOrder){
            order.second.first = miterAIG.fromNameToOrder(cir1.fromOrderToName(order.second.first));
        }
        for(auto &order : cir2NameToOrder){
            if(order.second.first == -1)continue;
            order.second.first = miterAIG.fromNameToOrder(cir2.fromOrderToName(order.second.first));
        }
        return  {{cir1NameToOrder, cir2NameToOrder}, counter};
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
    name.erase(name.begin());
    return {name, negative};
}
int tmpCounter = 0;
void
TwoStep::reduceSpace(CNF &mappingSpace, const vector<bool> &counter, const int baseLength, AIG &cir1, AIG &cir2,
                     const vector<MP> &mapping, pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>> &nameToOrder
#ifdef DBG
                        , const vector<MP> &R
#endif
                     ) {
    tmpCounter++;
    auto& [cir1NameToOrder, cir2NameToOrder] = nameToOrder;
    vector<bool> cir1Input, cir2Input;
    cir1Input.reserve(cir1.getInputNum());
    cir2Input.reserve(cir2.getInputNum());
    // TODO optimize multiple transfer
    for(auto i = 0 ; i < cir1.getInputNum() ; i++){
        auto order = cir1NameToOrder[cir1.fromOrderToName(i)];
        if(order.second){
            cir1Input.push_back(!counter[order.first]);
        }else{
            cir1Input.push_back(counter[order.first]);
        }
    }
    for(auto i = 0 ; i < cir2.getInputNum() ; i++){
        auto order = cir2NameToOrder[cir2.fromOrderToName(i)];
        if(order.first >= 0){
            if(order.second){
                cir2Input.push_back(!counter[order.first]);
            }else{
                cir2Input.push_back(counter[order.first]);
            }
        }else{
            if(order.first == -1){
                cir2Input.push_back(order.second);
            }
#ifdef DBG
            else{
                cout << "[TwoStep] Error: can not handle." << order.first << endl;
                exit(1);
            }
#endif
        }
    }
    bool notEqual = false;
    vector<bool> testCir1Output = cir1.generateOutput(cir1Input);
    vector<bool> testCir2Output = cir2.generateOutput(cir2Input);
    vector<bool> cir1Counter(cir1.getOutputNum(), false), cir2Counter(cir2.getOutputNum(), false);
    for(const auto& pair : R){
        auto [gateName, negative] = analysisName(pair.first);
        if((testCir1Output[cir1.fromNameToOrder(cir1.cirName + gateName) - cir1.getInputNum()] ^ negative) ^ testCir2Output[cir2.fromNameToOrder(pair.second) - cir2.getInputNum()]){
            cir1Counter[cir1.fromNameToOrder(cir1.cirName + gateName) - cir1.getInputNum()] = true;
            cir2Counter[cir2.fromNameToOrder(pair.second) - cir2.getInputNum()] = true;
            notEqual = true;
        }
    }
#ifdef DBG
    //TODO delete test
    if(!notEqual){
        if(verbose){
            cout << "cir1Input:" << cir1Input.size() << endl;
            for(auto && i : cir1Input){
                cout << i<<" ";
            }
            cout << endl;
            cout << "cir2Input: " << cir2Input.size() << endl;
            for(auto && i : cir2Input){
                cout << i<<" ";
            }
            cout << endl;
        }
        cout << "cir1Output:" << endl;
        for(auto i : testCir1Output){
            cout << i << " ";
        }
        cout << endl;
        cout << "cir2Output:" << endl;
        for(auto i : testCir2Output){
            cout << i << " ";
        }
        cout << endl;
        cout << "[TwoStep] SelfTest fail: cir1 == cir2!" << endl;
        exit(1);
    }
    for(auto i : mapping){
        if(i.first.size() == 1){
            if(cir2Input[cir2.fromNameToOrder(i.second)] != stoi(i.first)){
                cout << "SelfCheck failed:" << i.first << " " << i.second << endl;
            }
            continue;
        }
        auto [name, negative] = analysisName(i.first);
        if((cir1Input[cir1.fromNameToOrder(cir1.cirName + name)] ^ negative) != cir2Input[cir2.fromNameToOrder(i.second)]){
            cout << "[TwoStep] SelfCheck failed:" << i.first << " " << i.second << endl;
            exit(1);
        }
    }
#endif
    vector<int> cir1NonRedundant = getNonRedundant(cir1Input, cir1, cir1Counter)
            , cir2NonRedundant = getNonRedundant(cir2Input, cir2, cir2Counter); // return order
#ifdef DBG
    for(int i = 0 ; i < cir1.getInputNum() ; i++){
        vector<int> notEqualCnt(cir2.getOutputNum());
        vector<bool> testCir1Input = cir1Input;
        testCir1Input[i] = !testCir1Input[i];
        if((find(cir1NonRedundant.begin(), cir1NonRedundant.end(), i) == cir1NonRedundant.end())){
            auto oriOutput = cir1.generateOutput(cir1Input);
            auto testOutput = cir1.generateOutput(testCir1Input);
            for(int q = 0 ; q < cir1.getOutputNum() ; q++){
                if(!cir1Counter[q]){
                    notEqualCnt[q] = 10;
                    continue;
                }
                if((oriOutput[q] != testOutput[q])) {
                    notEqualCnt[q]++;
                }
            }
            if(find(notEqualCnt.begin(), notEqualCnt.end(), 0) == notEqualCnt.end()){
                cout << "[TwoStep] cir1 nonRedundant SelfTest1 failed." << i << endl;
                exit(1);
            }else if(!cir1Counter[find(notEqualCnt.begin(), notEqualCnt.end(), 0) - notEqualCnt.begin()]){
                cout << "Code Error" << endl;
                exit(1);
            }
        }else{
//            auto oriOutput = cir1.generateOutput(testCir1Input);
//            auto testOutput = cir1.generateOutput(testCir1Input);
//            for(int q = 0 ; q < cir1.getOutputNum() ; q++){
//                if(!cir1Counter[q])continue;
//                if((oriOutput[q] == testOutput[q])){
//                    cout << "[TwoStep] cir1 nonRedundant SelfTest2 failed." << i << endl;
//                    exit(1);
//                }
//            }
        }
    }
    for(int i = 0 ; i < cir2.getInputNum() ; i++){
        vector<int> notEqualCnt(cir2.getOutputNum());
        vector<bool> testCir2Input = cir2Input;
        testCir2Input[i] = !testCir2Input[i];
        if((find(cir2NonRedundant.begin(), cir2NonRedundant.end(), i) == cir2NonRedundant.end())){
            auto oriOutput = cir2.generateOutput(cir2Input);
            auto testOutput = cir2.generateOutput(testCir2Input);
            for(int q = 0 ; q < cir2.getOutputNum() ; q++){
                if(!cir2Counter[q]){
                    notEqualCnt[q] = 10;
                    continue;
                }
                if((oriOutput[q] != testOutput[q])) {
                    notEqualCnt[q]++;
                }
            }
            if(find(notEqualCnt.begin(), notEqualCnt.end(), 0) == notEqualCnt.end()){
                cout << "[TwoStep] cir2 nonRedundant SelfTest1 failed." << i << endl;
                exit(1);
            }else if(!cir2Counter[find(notEqualCnt.begin(), notEqualCnt.end(), 0) - notEqualCnt.begin()]){
                cout << "Code Error" << endl;
                exit(1);
            }
        }else{
//            auto oriOutput = cir2.generateOutput(testCir2Input);
//            auto testOutput = cir2.generateOutput(testCir2Input);
//            for(int q = 0 ; q < cir2.getOutputNum() ; q++){
//                if(!cir2Counter[q])continue;
//                if((oriOutput[q] != testOutput[q])){
//                    cout << "[TwoStep] cir2 nonRedundant SelfTest2 failed." << i << endl;
//                    exit(1);
//                }
//            }
        }
    }
#endif
    vector<int> clause;
    vector<string> record;
    if(verbose){
        cout << "cir1NonRedundant" << endl;
        for(auto i : cir1NonRedundant){
            cout << i << " ";
        }
        cout << endl;
        cout << "cir2NonRedundant" << endl;
        for(auto i : cir2NonRedundant){
            cout << i << " ";
        }
        cout << endl;
    }
    for(auto i : cir2NonRedundant){
        for(auto j : cir1NonRedundant){

//            cout <<"order:"<< i << " " << j << endl;
            if(cir1Input[j] == cir2Input[i]){
//                cout << "equal:" << i * baseLength + j * 2 + 1 + 1 << endl;
                clause.push_back(i * baseLength + j * 2 + 1 + 1);
                record.push_back(cir1.fromOrderToName(j) + '\'' + "_" + cir2.fromOrderToName(i));
            }else{
//                cout << "not equal:" << i * baseLength + j * 2 + 1 << endl;
                clause.push_back(i * baseLength + j * 2 + 1);
                record.push_back(cir1.fromOrderToName(j) + "_" + cir2.fromOrderToName(i));
            }
        }
        if(cir2Input[i]){
//            cout << "0:" << i * baseLength + cir1.getInputNum() * 2 + 1 << endl;
            clause.push_back(i * baseLength  + cir1.getInputNum() * 2 + 1);
            record.push_back("0_" + cir2.fromOrderToName(i));
        }else{
//            cout << "1:" << i * baseLength + cir1.getInputNum() * 2 + 1 + 1 << endl;
            clause.push_back(i * baseLength  + cir1.getInputNum() * 2 + 1 + 1);
            record.push_back("1_" + cir2.fromOrderToName(i));
        }
    }
//    cout << "clause:" << endl;
//    for(auto i : clause){
//        cout << i << " ";
//    }
//    cout << endl;
#ifdef DBG
    //TODO delete test
    for(const auto &i :mappingSpace.getClauses()){
        if(clause == i){
            cout << "[TwoStep] Error: can not find right clause cause infinite loop." << endl;
            exit(1);
        }
    }
#endif
    clauseStack.push_back(record);
    mappingSpace.addClause(clause);
    clause.clear();
    record.clear();
}


vector<int> TwoStep::getNonRedundant(const vector<bool> &input, AIG &cir, vector<bool> counter) {
    // TODO check need fix output that is different or just have different output
    vector<int> re;
    auto originOutput = cir.generateOutput(input);
    for(int i = 0 ; i < cir.getOutputNum() ; i++){
        if(!counter[i])continue;
        vector<int> fi;
        fi.reserve(cir.getInputNum());
        set<string> funSup = cir.getSupport(cir.fromOrderToName(cir.getInputNum() + i), 1);
        for (int p = 0; p < cir.getInputNum(); p++) {
            if(funSup.find(cir.fromOrderToName(p)) != funSup.end())continue;
            fi.push_back(p);
            vector<bool> input2 = input;
            for(auto q : fi){
                input2[q] = !input2[p];
                if (originOutput[i] != cir.generateOutput(input2)[i]) {
                    fi.pop_back();
                    break;
                }
            }
        }
        for (int p = 0; p < cir.getInputNum(); p++) {
            if (funSup.find(cir.fromOrderToName( p)) == funSup.end()){
                fi.push_back(p);
            }
        }
        if (fi.size() > re.size()) {
            re = fi;
        }
    }
    sort(re.begin(), re.end());
    vector<int> nf;
    int ptr = 0;
    for(int i = 0 ; i < cir.getInputNum() ; i++){
        while (ptr < static_cast<int>(re.size()) && re[ptr] < i){
            ptr++;
        }
        if(ptr < static_cast<int>(re.size()) && re[ptr] == i)continue;
        nf.push_back(i);
    }
    return nf;
}

void TwoStep::tsDebug(string msg, AIG cir1, AIG cir2) {
    if(!verbose)return;
    cout << msg << endl;
    cout << "Cir1" << endl;
    cout << cir1.getRaw();
    cout << "Cir2" << endl;
    cout << cir2.getRaw();
}

int TwoStep::recordOutput(const vector<MP> &inputMatch, const vector<MP> &R) {
    map<string, int> nameMap;
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
         << " iterations." << endl;
#endif
    parseOutput(outputFilePath, OutputStructure{inputGroups, outputGroups, one, zero}, matchOutput);
    return  matchOutput;
}


