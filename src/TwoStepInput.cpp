//
// Created by grorge on 7/22/23.
//
#include <utility>
#include "TwoStep.h"
#include "CNF.h"
#include "utility.h"
#include "parser.h"
#include "largeScale.h"

bool TwoStep::generateClause(CNF &mappingSpace, AIG &cir1Reduce, AIG &cir2Reduce, const vector<MP> &R,
                             bool outputProjection) {
    int baseLength = (cir1Reduce.getInputNum() + 1) * 2;
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
    // circuit 1 input must match
    for(int i = 0 ; i < cir1Reduce.getInputNum() ; i++){
        vector<int> clause;
        clause.reserve(2 * cir2Reduce.getInputNum());
        for(int q = 0 ; q < cir2Reduce.getInputNum() ; q++){
            clause.push_back(q * baseLength + i * 2 + 1);
            clause.push_back(q * baseLength + i * 2 + 1 + 1);
        }
        mappingSpace.addClause(clause);
        clause.clear();
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
        for (int i = 0; i < cir1Reduce.getInputNum() ; i++) {
            for(int u = 0 ; u < 2 ; u++){
                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                    for(int d = 0 ; d < 2 ; d++){
                        for (int k = q; k < cir2Reduce.getInputNum(); k++) {
                            if(k == q && d <= u)continue;
                            vector<int> clause;
                            clause.push_back((q * baseLength + i + u + 1) * -1);
                            clause.push_back((k * baseLength + i + d + 1) * -1);
                            mappingSpace.addClause(clause);
                        }
                    }
                }
            }
        }
        if(!outputProjection){
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
//            for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
//                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
//                    if (cir1Reduce.getSupport(cir1Reduce.fromOrderToName(i), 1).size() == cir2Reduce.getSupport(cir2Reduce.fromOrderToName(q), 1).size())continue;
////                    if (cir1Reduce.getSupport(cir1Reduce.fromOrderToName(i), 1).size() > cir2Reduce.getSupport(cir2Reduce.fromOrderToName(q), 1).size() + 3)continue;
//                    vector<int> clause;
//                    clause.push_back(-1 * (q * baseLength + 2 * i + 1));
//                    mappingSpace.addClause(clause);
//                    clause.clear();
//                    clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
//                    mappingSpace.addClause(clause);
//                }
//            }
        }
    }else if(cir1Reduce.getInputNum() < cir2Reduce.getInputNum()){
        if(!outputProjection){
            for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                    if (cir1Reduce.getSupport(cir1Reduce.fromOrderToName(i), 1).size() <= cir2Reduce.getSupport(cir2Reduce.fromOrderToName(q), 1).size())continue;
//                    if (cir1Reduce.getSupport(cir1Reduce.fromOrderToName(i), 1).size() > cir2Reduce.getSupport(cir2Reduce.fromOrderToName(q), 1).size() + 3)continue;
                    vector<int> clause;
                    clause.push_back(-1 * (q * baseLength + 2 * i + 1));
                    mappingSpace.addClause(clause);
                    clause.clear();
                    clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
                    mappingSpace.addClause(clause);
                }
            }
        }

    }else if(cir1Reduce.getInputNum() > cir2Reduce.getInputNum()){
        return false;
    }
    //remove funSupport not equal
//    //TODO add require constant and projection
//    LargeScale inputLg = LargeScale(cir1Reduce, cir2Reduce);
//    auto eigenValue = inputLg.calculateEigenvalue();
//    for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
//        for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
//            if (eigenValue[cir1Reduce.fromOrderToName(i)] == eigenValue[cir2Reduce.fromOrderToName(q)])continue;
//            vector<int> clause;
//            clause.push_back(-1 * (q * baseLength + 2 * i + 1));
//            mappingSpace.addClause(clause);
//            clause.clear();
//            clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
//            mappingSpace.addClause(clause);
//        }
//    }
    //Output Group Signature Heuristics
    for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
        for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
            bool equal = true;
            auto cir2Sup = cir2Reduce.getSupport(cir2Reduce.fromOrderToName(q), 1);
            for(const auto& supName: cir1Reduce.getSupport(cir1Reduce.fromOrderToName(i), 1)){
                bool find = false;
                for(const auto& pair : R){
                    auto [gateName, negative] = analysisName(pair.first);
                    if(cir1.cirName + gateName == supName && cir2Sup.find(pair.second) != cir2Sup.end()){
                        find = true;
                        break;
                    }
                }
                if(!find){
                    equal = false;
                }
            }
            if(equal)continue;
            vector<int> clause;
            clause.push_back(-1 * (q * baseLength + 2 * i + 1));
            mappingSpace.addClause(clause);
            clause.clear();
            clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
            mappingSpace.addClause(clause);
        }
    }
    return true;
}
vector<MP> TwoStep::inputSolver(vector<MP> &R, bool outputProjection) {
    //TODO define input solver stack recore input solver information
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
    if(!generateClause(mappingSpace, cir1Reduce, cir2Reduce, R, outputProjection)){
        return {};
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
    tsDebug("Reduce Network", cir1Reduce, cir2Reduce);
    auto [cir1BusPair, cir2BusPair]= generateBusMatchVector(cir1Reduce, cir2Reduce);
    auto cir1BusMatch = std::move(cir1BusPair.first);
    auto cir1BusCapacity = std::move(cir1BusPair.second);
    auto cir2BusMatch = std::move(cir2BusPair.first);
    auto cir2BusCapacity = std::move(cir2BusPair.second);
    set<pii> busMatch;
    vector<matchStatus> matchStack;
    auto popStack = [&]() -> int {
        auto &clauseRecordPoP = matchStack.back().clauseRecord;
        for(auto it : clauseRecordPoP){
            mappingSpace.eraseClause(it);
        }
        matchStack.pop_back();
        if(static_cast<int>(matchStack.size()) == 0){
            return 0;
        }
        auto &[cir1ChooseBusLast, cir2ChooseBusLast, minNumRecordLast, clauseRecord] = matchStack.back();
        cir2BusCapacity[cir2BusMatch[cir2ChooseBusLast]] += minNumRecordLast;
        cir1BusCapacity[cir1BusMatch[cir1ChooseBusLast]] += minNumRecordLast;
        if(verbose){
            cout << "unbind:" << cir1BusMatch[cir1ChooseBusLast] << " " << cir2BusMatch[cir2ChooseBusLast] << endl;
        }
#ifdef DBG
        if(busMatch.find(pii(cir1BusMatch[cir1ChooseBusLast], cir2BusMatch[cir2ChooseBusLast])) == busMatch.end()){
            cout << "[TwoStep] Error: match bus backtrace remove bus match failed." << cir1ChooseBusLast << " " << cir2ChooseBusLast << endl;
            exit(1);
        }
#endif
        busMatch.erase(pii(cir1BusMatch[cir1ChooseBusLast], cir2BusMatch[cir2ChooseBusLast]));
        cir2ChooseBusLast++;
        return static_cast<int>(matchStack.size());
    };
    matchStack.emplace_back(0, 0);
    while (true) {
#ifdef DBG
        if(matchStack.empty()){
            cout << "[TwoStep] Error: matchStack is empty." << endl;
            exit(1);
        }
#endif
        auto &[cir1ChooseBus, cir2ChooseBus, minNumRecord, clauseRecord] = matchStack.back();
#ifdef DBG
        if(cir1ChooseBus > static_cast<int>(cir1BusMatch.size())){
            cout << "[TwoStep] Error: cir1ChooseBus out limit." << endl;
            exit(1);
        }
#endif
        if(cir1ChooseBus == static_cast<int>(cir1BusMatch.size())){
            if(verbose){
                cout << "BusMatch:" << " " << busMatch.size() << endl;
                for(const auto &i : busMatch){
                    cout << i.first << " " << i.second << endl;
                }
            }
            vector<MP> mapping;
            for (int i = 0; i < cir1Reduce.getInputNum(); i++) {
                for (int q = 0; q < cir2Reduce.getInputNum(); q++) {
                    bool cir1InBus = (cir1BusMapping.find(cir1Reduce.fromOrderToName(i)) != cir1BusMapping.end());
                    bool cir2InBus = (cir1BusMapping.find(cir2Reduce.fromOrderToName(q)) != cir2BusMapping.end());
                    bool isMatch = (busMatch.find(pii(cir1BusMapping[cir1Reduce.fromOrderToName(i)], cir2BusMapping[cir2Reduce.fromOrderToName(q)])) != busMatch.end());
//                        cout << "Set clause:" << cir1Reduce.fromOrderToName(i) << " " << cir2Reduce.fromOrderToName(q) << " " << cir1InBus << " " << cir2InBus << " " <<  cir1BusMapping[cir1Reduce.fromOrderToName(i)]<< " " << cir2BusMapping[cir2Reduce.fromOrderToName(q)] <<
//                       " " << isMatch << " " << (q * baseLength + 2 * i + 1) << endl;
                    if (cir1InBus && cir2InBus){
                        if (isMatch)continue;
                    }else{
                        if(!cir1InBus && !cir2InBus)continue;
                        if(!cir1InBus && cir2BusCapacity[cir2BusMapping[cir2Reduce.fromOrderToName(q)]] > 0)continue;
                        if(!cir2InBus && cir1BusCapacity[cir1BusMapping[cir1Reduce.fromOrderToName(q)]] > 0)continue;
                    }
                    vector<int> clause;
                    clause.push_back(-1 * (q * baseLength + 2 * i + 1));
                    clauseRecord.emplace_back(mappingSpace.addClause(clause));
                    clause.clear();
                    clause.push_back(-1 * (q * baseLength + 2 * i + 1 + 1));
                    clauseRecord.emplace_back(mappingSpace.addClause(clause));
                }
            }
            while (true) {
                if (nowMs() - startMs > maxRunTime) {
                    return {};
                }
                mapping = solveMapping(mappingSpace, cir1Reduce, cir2Reduce, baseLength);
                if (mapping.empty()){
                    if(verbose)
                        cout << "No mapping result." << endl;
                    break;
                }
                pair<pair<map<string, pair<int, bool>>, map<string, pair<int, bool> > >, vector<bool> > counter = solveMiter(
                        mapping, R, cir1Reduce, cir2Reduce);
                if (counter.second.empty()) {
                    if (verbose) {
                        cout << "Find Mapping" << endl;
                        for (const auto &pair: mapping) {
                            cout << pair.first << " " << pair.second << endl;
                        }
                        recordMs();
                    }
                    break;
                }
//                    if (verbose) {
//                        cout << "It is counterexample size:" << counter.second.size() << endl;
//                        recordMs();
//                    }
                reduceSpace(mappingSpace, counter.second, baseLength, cir1Reduce, cir2Reduce, mapping, counter.first,
                            R);
            }
            if(!mapping.empty()){
                return mapping;
            }
            popStack();
        }else{
            bool brFlag = false;
            for( ; cir2ChooseBus < static_cast<int>(cir2BusMatch.size()) ; cir2ChooseBus++){
                if(cir2BusCapacity[cir2BusMatch[cir2ChooseBus]] > 0){
                    int cir1busIdx = cir1BusMatch[cir1ChooseBus], cir2busIdx = cir2BusMatch[cir2ChooseBus];
                    if(verbose){
                        cout << "bind:" << cir1busIdx << " " << cir2busIdx << endl;
                    }
#ifdef DBG
                    if(cir1BusCapacity.find(cir1busIdx) == cir1BusCapacity.end() || cir2BusCapacity.find(cir2busIdx) == cir2BusCapacity.end()){
                        cout << "cir1BusCapacity:" << endl;
                        for(auto i : cir1BusCapacity){
                            cout << i.first << " " << i.second << endl;
                        }
                        cout << "cir2BusCapacity:" << endl;
                        for(auto i : cir2BusCapacity){
                            cout << i.first << " " << i.second << endl;
                        }
                        cout << "[TwoStep] Error: bus capacity can not found." << endl;
                        exit(1);
                    }
#endif
                    int minNum = min(cir1BusCapacity[cir1busIdx], cir2BusCapacity[cir2busIdx]);
                    cir2BusCapacity[cir2busIdx] -= minNum;
                    cir1BusCapacity[cir1busIdx] -= minNum;
                    busMatch.insert(pii(cir1busIdx, cir2busIdx));
                    minNumRecord = minNum;
                    if(cir1BusCapacity[cir1busIdx] == 0){
                        matchStack.emplace_back(cir1ChooseBus + 1, 0);
                    }else{
                        matchStack.emplace_back(cir1ChooseBus, cir2ChooseBus + 1);
                    }
                    brFlag = true;
                    break;
                }
            }
            if(!brFlag){
                if(cir2ChooseBus == static_cast<int>(cir2BusMatch.size())){
                    if(!popStack()){
                        return {};
                    }
                }
            }
        }
    }
}

pair<pair<vector<int>, map<int, int>>, pair<vector<int>, map<int, int>>>
TwoStep::generateBusMatchVector(AIG &cir1, AIG &cir2) {
    map<int, int> cir1BusCapacity, cir2BusCapacity;
    for(int i = 0 ; i < cir1.getInputNum() ; i++){
        if(cir1BusMapping.find(cir1.fromOrderToName(i)) == cir1BusMapping.end())continue;
        int busIdx = cir1BusMapping[cir1.fromOrderToName(i)];
        if(verbose)
            cout << "bus mapping:" << cir1.fromOrderToName(i) << " " << busIdx << endl;
//        cir1BusCapacity[busIdx] = static_cast<int>(cir1InputBus[busIdx].size());
        cir1BusCapacity[busIdx] = 10;
    }
    for(int i = 0 ; i < cir2.getInputNum() ; i++){
        if(cir2BusMapping.find(cir2.fromOrderToName(i)) == cir2BusMapping.end())continue;
        int busIdx = cir2BusMapping[cir2.fromOrderToName(i)];
        if(verbose)
            cout << "bus mapping:" << cir2.fromOrderToName(i) << " " << busIdx << endl;
//        cir2BusCapacity[busIdx] = static_cast<int>(cir2InputBus[busIdx].size());
        cir2BusCapacity[busIdx] = 10;
    }

    vector<int> cir1Ve, cir2Ve;
    for(const auto &match : cir1BusCapacity){
        if(match.second > 0){
            cir1Ve.emplace_back(match.first);
        }
    }
    for(const auto &match : cir2BusCapacity){
        if(match.second > 0){
            cir2Ve.emplace_back(match.first);
        }
    }
    if(verbose){
        cout << "cir1Ve:" << endl;
        for(auto i : cir1Ve){
            cout << i << " " << cir1BusCapacity[i] << endl;
        }
        cout << "cir2Ve:" << endl;
        for(auto i : cir2Ve){
            cout << i << " " << cir2BusCapacity[i] << endl;
        }
    }
    return {{cir1Ve, cir1BusCapacity}, {cir2Ve, cir2BusCapacity}};
}

vector<MP> TwoStep::solveMapping(CNF &mappingSpace, AIG &cir1, AIG &cir2, const int baseLength) {
    vector<MP> re;
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