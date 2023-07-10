//
// Created by grorge on 6/25/23.
//
#include <chrono>
#include <utility>
#include "TwoStep.h"
#include "CNF.h"
#include "utility.h"
#include "parser.h"

void generate_combinations(int index, vector<int>& input, vector<vector<bool>>& output){
    if(index == static_cast<int>(input.size())){
        vector<bool> temp(input.begin(), input.end());
        output.push_back(temp);
        return;
    }

    if(input[index] == 2){
        input[index] = 0;
        generate_combinations(index + 1, input, output);
        input[index] = 1;
        generate_combinations(index + 1, input, output);
        input[index] = 2;
    } else {
        generate_combinations(index + 1, input, output);
    }
}

vector<vector<bool>> convert_pair(vector<int> input){
    vector<vector<bool>> output;
    generate_combinations(0, input, output);
    return output;
}

TwoStep ts;
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
                if(R.size() == 0){
                    optimal = true;
                    break;
                }
                R.pop_back();
                outputSolverPop();
                projection = false;
            }
            continue;
        }
//        cout << "Output Pair:" << projection << endl;
//        recordMs();
//        for(const auto& pair : R){
//            cout << pair.first << ' ' << pair.second << endl;
//        }
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
            for(const auto& pair : R){
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
            for(const auto& pair : inputMatch){
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
#ifdef INF
            cout << "Two step matching port number: " << matchOutput << "(" << (float)matchOutput / (float)allOutputNumber * 100 << "%)" << endl;
#endif
            parseOutput(outputFilePath, OutputStructure{inputGroups, outputGroups, one, zero}, matchOutput);
            if( matchOutput == allOutputNumber)optimal = true;
        }
        if(nowMs() - startMs > maxRunTime){
            timeout = true;
        }
//        if(iteratorCounter == 0)break;
        iteratorCounter++;
    }
}

int TwoStep::nowMs() {
    return static_cast<int>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
}
int recordCounter = 0;
void TwoStep::recordMs() {
    return;
    int now = nowMs();
    cout <<"During:"<< now - lastTime << " Iteration:" << recordCounter << endl;
    recordCounter++;
    lastTime = now;
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
        if(pair.second.back() == '\''){
            pair.second.pop_back();
        }
        cir2ChoosePort.insert(pair.second);
    }
    vector<string> cir1Erase, cir2Erase;
    auto eraseNonUsedPort = [](AIG &cir, set<string> &cirChoosePort) -> vector<string>{
        set<string> cirErase;
        for(int i = cir.getInputNum() ; i < cir.getInputNum() + cir.getOutputNum(); i++){
            if(cirChoosePort.find(cir.fromOrderToName(i)) == cirChoosePort.end()){
                cirErase.insert(cir.fromOrderToName(i));
            }
        }
        for(int order = 0 ; order < cir.getInputNum() ; order++){
            bool eraseFlag = true;
            for(const string& funSupportPort: cir.getSupport(cir.fromOrderToName(order), 1)){
                if(cirErase.find(funSupportPort) == cirErase.end()){
                    eraseFlag = false;
                    break;
                }
            }
            if(eraseFlag){
                cirErase.insert(cir.fromOrderToName(order));
            }
        }
        vector<string> re;
        re.reserve(cirErase.size());
        for(const auto& erase : cirErase){
            re.push_back(erase);
        }
        return re;
    };
    cir1Erase = eraseNonUsedPort(cir1, cir1ChoosePort);
    cir2Erase = eraseNonUsedPort(cir2, cir2ChoosePort);
    AIG cir1Reduce = cir1;
    AIG cir2Reduce = cir2;
    cir1Reduce.erasePort(cir1Erase);
    cir2Reduce.erasePort(cir2Erase);
    CNF mappingSpace;
    mappingSpaceLast = 0;
    int baseLength = (cir1Reduce.getInputNum() + 1) * 2;
    mappingSpace.maxIdx = cir2Reduce.getInputNum() * baseLength;
    for(int i = 0 ; i < cir2Reduce.getInputNum() ; i++){
        for(int q = 0 ; q < cir1Reduce.getInputNum(); q ++){
            // CNF base 1
            mappingSpace.varMap[cir1Reduce.fromOrderToName(q) + "_" + cir2Reduce.fromOrderToName(i)] = i * baseLength + q * 2 + 1;
            mappingSpace.varMap[cir1Reduce.fromOrderToName(q) + '\'' + "_" + cir2Reduce.fromOrderToName(i)] = i * baseLength + q * 2 + 1 + 1;
        }
    }
    for(int i = 0 ; i < cir2Reduce.getInputNum() ; i++){
        mappingSpace.varMap["0_" + cir2Reduce.fromOrderToName(i)] = i * baseLength + cir1Reduce.getInputNum() * 2 + 1;
        mappingSpace.varMap["1_" + cir2Reduce.fromOrderToName(i)] = i * baseLength + cir1Reduce.getInputNum() * 2 + 1 + 1;
    }
    for(int i = 0 ; i < cir2Reduce.getInputNum() ; i++){
        vector<int> clause;
        clause.reserve(baseLength);
        for(int q = 0 ; q < baseLength ; q++){
            clause.push_back(i * baseLength + q + 1);
        }
        mappingSpace.clauses.emplace_back(clause);
        clause.clear();
        for(int q = 0 ; q < baseLength ; q++){
            for(int k = q + 1 ; k < baseLength ; k++){
                clause.push_back((i * baseLength + q + 1) * -1);
                clause.push_back((i * baseLength + k + 1) * -1);
                mappingSpace.clauses.emplace_back(clause);
                clause.clear();
            }
        }
    }
    if(cir1Reduce.getInputNum() == cir2Reduce.getInputNum()){
        //disable match constant
        for(int i = 0 ; i < cir2Reduce.getInputNum() ; i++){
            vector<int> clause;
            clause.push_back(-1 * (i * baseLength + cir1Reduce.getInputNum() * 2 + 1));
            mappingSpace.clauses.push_back(clause);
            clause.clear();
            clause.push_back(-1 * (i * baseLength + cir1Reduce.getInputNum() * 2 + 1 + 1));
            mappingSpace.clauses.push_back(clause);
        }
        // disable input projection
        for(int i = 0 ; i < cir1Reduce.getInputNum() * 2 ; i++){
            vector<int> clause;
            for(int q = 0 ; q < cir2Reduce.getInputNum() ; q++){
                for(int k = q + 1 ; k < cir2Reduce.getInputNum() ; k++){
                    clause.push_back((q * baseLength + i + 1) * -1);
                    clause.push_back((k * baseLength + i + 1) * -1);
                    mappingSpace.clauses.emplace_back(clause);
                    clause.clear();
                }
            }
        }
        for(int i = 0 ; i < cir1Reduce.getInputNum() ; i++){
            vector<int> clause;
            clause.reserve(2 * cir2Reduce.getInputNum());
            for(int q = 0 ; q < cir2Reduce.getInputNum() ; q++){
                clause.push_back(q * baseLength + 2 * i + 1);
                clause.push_back(q * baseLength + 2 * i + 1 + 1);
            }
            mappingSpace.clauses.emplace_back(clause);
            clause.clear();
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
                exit(1);
            }
#endif
            clause.emplace_back(mappingSpace.varMap[key]);
        }
        mappingSpace.clauses.push_back(clause);
        clause.clear();
    }
    vector<MP> mapping;
    tsDebug("Reduce Network", cir1Reduce, cir2Reduce);
    while (true){
        mapping = solveMapping(mappingSpace, cir1Reduce, cir2Reduce, baseLength);
        if(mapping.empty())break;
        // TODO delete this
//        cout << "Find Mapping" << endl;
//        for(auto pair : mapping){
//            cout << pair.first << " " << pair.second << endl;
//        }
        recordMs();
        pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool> > >, vector<vector<bool> > > counter = solveMiter(mapping, R, cir1Reduce, cir2Reduce);
        if(counter.second.empty()){
            break;
        }
//        cout << "It is counterexample size:" << counter.second.size() << endl;
        recordMs();
        for(auto counterexample : counter.second){
            reduceSpace(mappingSpace, counterexample, baseLength, cir1Reduce, cir2Reduce, mapping, counter.first, R);
        }
    }
    return mapping;
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
    ofstream of;
    string fileName = mappingSpaceFileName;
    if(mappingSpaceLast != 0){
        std::streampos firstLineEnd;
        std::fstream file;
        file.open(fileName, std::ios::in);
        std::string firstLine;
        std::getline(file, firstLine);
        firstLineEnd = file.tellg();
        file.close();
        file.open(fileName, std::ios::out | std::ios::in);
        string newLine = "p cnf " + to_string(mappingSpace.maxIdx) + " " + to_string(mappingSpace.clauses.size());
        if (newLine.length() <= firstLine.length()) {
            file << newLine;
            for (size_t i = newLine.length(); i < firstLine.length(); ++i) {
                file.put(' ');
            }
        } else {
#ifdef DBG
            cout << "[TwoStep] Error: cnf saveSpace not enough !" << endl;
            exit(1);
#endif
        }
        file.close();
        of.open(fileName, ios::app);
        string content;
        for(int i = mappingSpaceLast ; i < static_cast<int>(mappingSpace.clauses.size()) ; i++){
            for(int q : mappingSpace.clauses[i]){
                content += to_string(q) + " ";
            }
            content += "0\n";
        }
        of << content;
        of.close();
    }else{
        of.open(fileName);
        of << mappingSpace.getRaw();
        of.close();
    }
    mappingSpaceLast = static_cast<int>(mappingSpace.clauses.size());
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
                if(result.input[i * baseLength + q] > 0){
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
        free(result.input);
    }
    return re;
}

pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>>, vector<vector<bool>>>
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
        vector<int> counter;
        counter.resize(miterAIG.getInputNum());
        vector<bool> testCir1Input(cir1.getInputNum(), false), testCir2Input(cir2.getInputNum(), false);
        vector<bool> testMiterInput(miterAIG.getInputNum(), false);
        for(int order = 0 ; order < miterAIG.getInputNum() ; order++){
            string name = miterAIG.fromOrderToName(order);
            if(miter.isDC(name)){
                counter[order] = 2;
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
        return  {{cir1NameToOrder, cir2NameToOrder}, convert_pair(counter)};
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
//    cout << "cir1Input" << endl;
//    for(int i = 0 ; i < cir1Input.size() ; i++){
//        cout << cir1Input[i]<<" ";
//    }
//    cout << endl;
//    cout << "cir2Input" << endl;
//    for(int i = 0 ; i < cir2Input.size() ; i++){
//        cout << cir2Input[i]<<" ";
//    }
//    cout << endl;
#ifdef DBG
    //TODO delete test

    bool notEqual = false;
    vector<bool> testCir1Output = cir1.generateOutput(cir1Input);
    vector<bool> testCir2Output = cir2.generateOutput(cir2Input);
    for(int i = cir1.getInputNum() ; i < cir1.getInputNum() + cir1.getOutputNum() ; i++){
        string name = cir1.fromOrderToName(i);
        for(auto pair : R){
            auto [gateName, negative] = analysisName(pair.first);
            if((testCir1Output[cir1.fromNameToOrder(cir1.cirName + gateName) - cir1.getInputNum()] ^ negative) != testCir2Output[cir2.fromNameToOrder(pair.second) - cir2.getInputNum()]){
                notEqual = true;
            }
        }
    }
    if(!notEqual){
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
    vector<int> cir1NonRedundant = getNonRedundant(cir1Input, cir1), cir2NonRedundant = getNonRedundant(cir2Input, cir2); // return order
    vector<int> clause;
    vector<string> record;
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
    for(const auto &i :mappingSpace.clauses){
        if(clause == i){
            cout << "[TwoStep] Error: can not find right clause cause infinite loop." << endl;
            exit(1);
        }
    }
#endif
    clauseStack.push_back(record);
    mappingSpace.clauses.push_back(clause);
    clause.clear();
    record.clear();
}


vector<int> TwoStep::getNonRedundant(const vector<bool> &input, AIG &cir) {
    //TODO need to fix
    vector<int> re;
    for(int i = 0 ; i < cir.getInputNum() ; i++){
        re.push_back(i);
        vector<bool> input2 = input;
        input2[i] = !input2[i];
        // TODO check need fix output that is different or just have different output
        if(cir.generateOutput(input) == cir.generateOutput(input2)){
//            re.pop_back();
        }
    }
    return re;
}

void TwoStep::tsDebug(string msg, AIG cir1, AIG cir2) {
    cout << msg << endl;
    cout << "Cir1" << endl;
    cout << cir1.getRaw();
    cout << "Cir2" << endl;
    cout << cir2.getRaw();
}


