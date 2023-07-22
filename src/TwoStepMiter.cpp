//
// Created by grorge on 7/22/23.
//
#include <utility>
#include "TwoStep.h"
#include "CNF.h"
#include "utility.h"
#include "parser.h"

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
//    if(verbose){
//        cout << "cir1NonRedundant" << endl;
//        for(auto i : cir1NonRedundant){
//            cout << i << " ";
//        }
//        cout << endl;
//        cout << "cir2NonRedundant" << endl;
//        for(auto i : cir2NonRedundant){
//            cout << i << " ";
//        }
//        cout << endl;
//    }
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
//    if(!verbose)
    return;
    cout << msg << endl;
    cout << "Cir1" << endl;
    cout << cir1.getRaw();
    cout << "Cir2" << endl;
    cout << cir2.getRaw();
}