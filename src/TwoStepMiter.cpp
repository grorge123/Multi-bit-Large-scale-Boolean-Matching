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

CNF TwoStep::generateMiter(const vector<MP> &outputMatchPair, AIG cir1, AIG cir2) {
    set<string> projectiveOutput;
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
    vector<string> cir1NameArray;
    vector<string> cir2NameArray;
    for(int i = 0 ; i < cir1.getInputNum() ; i++){
         cir2NameArray.emplace_back(cir1.fromOrderToName(i));
    }
    for(int i = 0 ; i < cir2.getInputNum() ; i++){
        cir1NameArray.emplace_back(cir2.fromOrderToName(i));
    }
    cir1.addFloatInput(cir1NameArray);
    cir2.addFloatInput(cir2NameArray);
    CNF miter;
    AIG miterAIG;
    ::solveMiter(cir1, cir2, miter, miterAIG);
    return miter;
}

pair<vector<bool>, vector<bool>>
TwoStep::solveMiter(const vector<MP> &inputMatchPair, CNF originMiter, AIG &cir1, AIG &cir2) {
    CNF miter;
    originMiter.copy(miter);
    for(const auto &pair : inputMatchPair){
#ifdef DBG
        if(miter.varMap.find(pair.second) == miter.varMap.end()){
            cout << "[TwoStepMiter] Error: miter cant not found cir2 port." << endl;
            exit(1);
        }
#endif
        if(pair.first == "0"){
            miter.addClause({-1 * miter.varMap[pair.second]});
        }else if(pair.first == "1"){
            miter.addClause({miter.varMap[pair.second]});
        }else{
            auto [gateName, negation] = analysisName(pair.first);
#ifdef DBG
            if(miter.varMap.find(cir1.cirName + gateName) == miter.varMap.end()){
                cout << "[TwoStepMiter] Error: miter cant not found cir1 port. " << cir1.cirName + gateName << endl;
                exit(1);
            }
#endif
            int A = miter.varMap[cir1.cirName + gateName];
            int B = miter.varMap[pair.second];
            if(negation){
                miter.addClause({A, B});
                miter.addClause({-1 * A, -1 * B});
            }else{
                miter.addClause({A, -1 * B});
                miter.addClause({-1 * A, B});
            }
        }
    }

    miter.solve();
    vector<bool> cir1Input(cir1.getInputNum());
    vector<bool> cir2Input(cir2.getInputNum());
    if(miter.satisfiable){
        for(int i = 0 ; i < cir1.getInputNum() ; i++){
            string name = cir1.fromOrderToName(i);
#ifdef DBG
            if(miter.varMap.find(name) == miter.varMap.end()){
                cout << "[TwoStepMiter] Error: miter can not found cir1 after solve." << endl;
                exit(1);
            }
#endif
            cir1Input[i] = miter.satisfiedInput[miter.varMap[name] - 1];
        }
        for(int i = 0 ; i < cir2.getInputNum() ; i++){
            string name = cir2.fromOrderToName(i);
#ifdef DBG
            if(miter.varMap.find(name) == miter.varMap.end()){
                cout << "[TwoStepMiter] Error: miter can not found cir1 after solve." << endl;
                exit(1);
            }
#endif
            cir2Input[i] = miter.satisfiedInput[miter.varMap[name] - 1];
        }
#ifdef DBG
        bool fail = false;
        for(const auto &pair: inputMatchPair){
            if(pair.first.size() == 1){
                if(stoi(pair.first) != cir2Input[cir2.fromNameToOrder(pair.second)]){
                    cout <<"A:"<< pair.first << " " << pair.second << " " << pair.first << " " << cir2Input[cir2.fromNameToOrder(pair.second)] << endl;
                    fail = true;
                }
            }else{
                auto [gateName, negation] = analysisName(pair.first);
                if(negation){
                    if(cir1Input[cir1.fromNameToOrder(cir1.cirName + gateName)] == cir2Input[cir2.fromNameToOrder(pair.second)]){
                        cout << "B:" << pair.first << " " << pair.second << " " << cir1Input[cir1.fromNameToOrder(cir1.cirName + gateName)] << " " << cir2Input[cir2.fromNameToOrder(pair.second)] << endl;
                        fail = true;
                    }
                }else{
                    if(cir1Input[cir1.fromNameToOrder(cir1.cirName + gateName)] != cir2Input[cir2.fromNameToOrder(pair.second)]){
                        cout << "C:" << pair.first << " " << pair.second << " " << cir1Input[cir1.fromNameToOrder(cir1.cirName + gateName)] << " " << cir2Input[cir2.fromNameToOrder(pair.second)] << endl;
                        fail = true;
                    }
                }
            }
        }
        if(fail){
            cout << "[TwoStepMiter] Error: selfTest miter assign failed. " << endl;
            exit(1);
        }
#endif
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
    name.erase(name.begin());
    return {name, negative};
}


pair<vector<vector<int>>, vector<vector<int>>>
TwoStep::reduceSpace(CNF &mappingSpace, const int baseLength, AIG &cir1, AIG &cir2, const vector<MP> &mapping,
                     const vector<MP> &R, const vector<bool> &cir1Input, const vector<bool> &cir2Input) {

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
        if (!cir1Counter[cir1CounterIdx] || !cir2Counter[cir2CounterIdx]) {
            continue;
        }
        auto cir1NR = getNonRedundant2(cir1Input, cir1, cir1CounterIdx);
#ifdef DBG
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
#endif
        cir1NRSet.emplace_back(std::move(cir1NR));
        auto cir2NR = getNonRedundant2(cir2Input, cir2, cir2CounterIdx);
#ifdef DBG
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
#endif
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
//        cout << 0 << endl;
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
//        cout << "[TwoStep] Error: can not find right clause cause infinite loop." << endl;
//        exit(1);
    }
#endif
    return {cir1NRSet, cir2NRSet};
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

vector<int> TwoStep::getNonRedundant2(const vector<bool> &input, AIG &cir, int counterIdx) {
    auto originOutput = cir.generateOutput(input);
    set<string> funSup = cir.getSupport(cir.fromOrderToName(cir.getInputNum() + counterIdx), 2);
    vector<string> removeVe;
    for(int i = 0 ; i < cir.getInputNum() ; i++){
        if(funSup.find(cir.fromOrderToName(i)) == funSup.end()){
            removeVe.emplace_back(cir.fromOrderToName(i));
        }
    }
    for(int i = cir.getInputNum() ; i < cir.getInputNum() + cir.getOutputNum() ; i++){
        if(i == counterIdx + cir.getInputNum())continue;
        removeVe.emplace_back(cir.fromOrderToName(i));
    }
    AIG cirCopy = cir;
    cirCopy.erasePort(removeVe);
#ifdef DBG
    if(cirCopy.getOutputNum() != 1 || cirCopy.getInputNum() != static_cast<int>(funSup.size())){
        cout << "[TwoStepMiter] Error: cirCopy incorrect." << endl;
        exit(1);
    }
#endif
    if(originOutput[counterIdx]){
        cirCopy.invertGate(cirCopy.fromOrderToName(cirCopy.getInputNum()));
    }
    CNF ntk(cirCopy);
    set<string> f;
    vector<int> nf;
    for (int p = 0; p < cirCopy.getInputNum(); p++) {
        if(ntk.varMap.find(cirCopy.fromOrderToName(p)) == ntk.varMap.end()){
            continue;
        }
        f.insert(cirCopy.fromOrderToName(p));
        for(int q = p + 1 ; q < cirCopy.getInputNum() ; q++){
            string name = cirCopy.fromOrderToName(q);
            if(ntk.varMap.find(name) != ntk.varMap.end()){
                if(input[cir.fromNameToOrder(name)]){
                    ntk.addAssume(ntk.varMap[name]);
                }else{
                    ntk.addAssume(-1 * ntk.varMap[name]);
                }
            }
        }
        ntk.solve();
        if(ntk.satisfiable){
            string name = cirCopy.fromOrderToName(p);
            if(input[cir.fromNameToOrder(name)]){
                ntk.addClause({ntk.varMap[name]});
            }else{
                ntk.addClause({-1 * ntk.varMap[name]});
            }
            nf.push_back(cir.fromNameToOrder(name));
        }
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