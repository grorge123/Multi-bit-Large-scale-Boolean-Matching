//
// Created by grorge on 7/22/23.
//
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
//                if(verbose){
//                    cout << "miter name :" << name << " " << counter[order] << endl;
//                }
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
//        if(verbose){
//            cout << "testCir1Input:" << endl;
//            for(int i = 0 ; i < static_cast<int>(testCir1Input.size()) ; i++){
//                cout << testCir1Input[i] << " ";
//            }
//            cout << endl;
//            cout << "testCir2Input:" << endl;
//            for(int i = 0 ; i < static_cast<int>(testCir2Input.size()) ; i++){
//                cout << testCir2Input[i] << " ";
//            }
//            cout << endl;
//        }
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


void TwoStep::reduceSpace(CNF &mappingSpace, const vector<bool> &counter, const int baseLength, AIG &cir1, AIG &cir2,
                          const vector<MP> &mapping, pair<map<string, pair<int, bool>>, map<string, pair<int, bool>>> &nameToOrder, const vector<MP> &R) {
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
//        if(verbose){
//            cout << "cir1Input:" << cir1Input.size() << endl;
//            for(auto && i : cir1Input){
//                cout << i<<" ";
//            }
//            cout << endl;
//            cout << "cir2Input: " << cir2Input.size() << endl;
//            for(auto && i : cir2Input){
//                cout << i<<" ";
//            }
//            cout << endl;
//        }
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
    for(const auto& i : mapping){
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
    vector<vector<int> > cir1NRSet, cir2NRSet;
    vector<int> cir1MaxNRSet, cir2MaxNRSet;
    set<pair<int, vector<int>> > clauseSet;
    vector<vector<string> > recordVe;
    for(const auto & pair : R) {
        int cir1CounterIdx = cir1.fromNameToOrder(cir1.cirName + analysisName(pair.first).first) - cir1.getInputNum();
        int cir2CounterIdx = cir2.fromNameToOrder(pair.second) - cir2.getInputNum();
#ifdef DBG
        if(cir1CounterIdx >= static_cast<int>(cir1Counter.size()) && cir2CounterIdx >= static_cast<int>(cir2Counter.size())){
            cout << "[TwoStepMiter] Error: get wrong counter Idx." << endl;
            exit(1);
        }
#endif
        if (!cir1Counter[cir1CounterIdx] && !cir2Counter[cir2CounterIdx]) {
            continue;
        }
        auto cir1NR = getNonRedundant(cir1Input, cir1, cir1CounterIdx);
//#ifdef DBG
//        {
//            auto original = cir1.generateOutput(cir1Input);
//            vector<int> input2(cir1Input.size(), 2);
//            for (auto q: cir1NR) {
//                input2[q] = cir1Input[q];
//            }
//            auto allTest = convert_pair(input2);
//            for (const auto &testCase: allTest) {
//                if (original[cir1CounterIdx] != cir1.generateOutput(testCase)[cir1CounterIdx]) {
//                    cout << "[TwoStepMiter] Error: selfTest fail. NR1 is wrong." << endl;
//                    exit(1);
//                }
//            }
//        }
//#endif
        cir1NRSet.emplace_back(std::move(cir1NR));
        auto cir2NR = getNonRedundant(cir2Input, cir2, cir2CounterIdx);
//#ifdef DBG
//        {
//            auto original = cir2.generateOutput(cir2Input);
//            vector<int> input2(cir2Input.size(), 2);
//            for(auto q : cir2NR){
//                input2[q] = cir2Input[q];
//            }
//            auto allTest = convert_pair(input2);
//            for(const auto& testCase : allTest){
//                if(original[cir2CounterIdx] != cir2.generateOutput(testCase)[cir2CounterIdx]){
//                    cout << "[TwoStepMiter] Error: selfTest fail. NR2 is wrong." << endl;
//                    exit(1);
//                }
//            }
//        }
//#endif
        cir2NRSet.emplace_back(std::move(cir2NR));
    }
    for(int idx = 0 ; idx < static_cast<int> (cir1NRSet.size()) ; idx++){
        const auto &cir1NonRedundant = cir1NRSet[idx];
        const auto &cir2NonRedundant = cir2NRSet[idx];
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
            if(cir2Input[i]){
                clause.push_back(i * baseLength  + cir1.getInputNum() * 2 + 1);
                record.push_back("0_" + cir2.fromOrderToName(i));
            }else{
                clause.push_back(i * baseLength  + cir1.getInputNum() * 2 + 1 + 1);
                record.push_back("1_" + cir2.fromOrderToName(i));
            }
        }
//        cout << "clause:" << endl;
//        for(auto i : clause){
//            cout << i << " ";
//        }
//        cout << endl;
        clauseSet.insert({recordVe.size(), clause});
        recordVe.push_back(record);
    }
#ifdef DBG
    bool infinite = true;
#endif
    for(const auto& clause : clauseSet){
#ifdef DBG
        //TODO delete test
        bool find = false;
        for(const auto &i :mappingSpace.getClauses()){
            if(clause.second == i){
                find = true;
            }
        }
        if(!find)infinite = false;
#endif
        clauseStack.push_back(recordVe[clause.first]);
        mappingSpace.addClause(clause.second);
    }
#ifdef DBG
    if(infinite){
        cout << "[TwoStep] Error: can not find right clause cause infinite loop." << endl;
        exit(1);
    }
#endif
}
vector<int> TwoStep::getNonRedundant(const vector<bool> &input, AIG &cir, int counterIdx) {
    auto originOutput = cir.generateOutput(input);
    vector<int> fi;
    fi.reserve(cir.getInputNum());
    set<string> funSup = cir.getSupport(cir.fromOrderToName(cir.getInputNum() + counterIdx), 1);
    for (int p = 0; p < cir.getInputNum(); p++) {
        if(funSup.find(cir.fromOrderToName(p)) == funSup.end())continue;
        fi.push_back(p);
        vector<int> input2(input.size());
        for(int i = 0 ; i < static_cast<int>(input.size()) ; i++){
            input2[i] = input[i];
        }
        for(auto q : fi){
            input2[q] = 2;
        }
        auto allTest = convert_pair(input2);
        for(const auto &test : allTest){
            if (originOutput[counterIdx] != cir.generateOutput(test)[counterIdx]) {
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
    sort(fi.begin(), fi.end());
    vector<int> nf;
    int ptr = 0;
    for(int i = 0 ; i < cir.getInputNum() ; i++){
        while (ptr < static_cast<int>(fi.size()) && fi[ptr] < i){
            ptr++;
        }
        if(ptr < static_cast<int>(fi.size()) && fi[ptr] == i)continue;
        nf.push_back(i);
    }
    return nf;
}

void TwoStep::tsDebug(string msg, AIG cir1, AIG cir2) {
//    if(!verbose)
    return;
    cout << msg << endl;
    cout << "Cir1" << endl;
    cout << cir1.getRaw();
    cout << "Cir2" << endl;
    cout << cir2.getRaw();
}