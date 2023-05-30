//
// Created by Han Wen Tsao on 2023/5/19.
//

#include "largeScale.h"
#include "utility.h"
#include "aiger.h"
#include "aigtocnf.h"
#include "satsolver.h"
#include <fstream>

using namespace std;

void LargeScale::randomSimulation(int only) {
    int stopNum = 10;
    int noChangeNum = stopNum;
    while (noChangeNum > 0){
        int change = 0;
        pair<vector<bool>, vector<bool>> input = generateInput();
        vector<bool> output1 = cir1.generateOutput(input.first);
        vector<bool> output2 = cir2.generateOutput(input.second);
        vector<vector<bool>> outputVector1;
        vector<vector<bool>> outputVector2;
        vector<vector<bool> > st1Record1, st1Record2;
        if(!only || only == 1)change += cir1.simulationType1(output1, st1Record1);
        if(!only || only == 1)change += cir2.simulationType1(output2, st1Record2);
        dealRecord(st1Record1, st1Record2, false);
        for(unsigned int i = 0 ; i < input.first.size() ; i++){
            vector<bool> tmpInput;
            for(unsigned int q = 0 ; q < input.first.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input.first[q]);
                }else{
                    tmpInput.push_back(input.first[q]);
                }
            }
            vector<bool> tmpOutput1 = cir1.generateOutput(tmpInput);
            outputVector1.push_back(tmpOutput1);
        }
        for(unsigned int i = 0 ; i < input.second.size() ; i++){
            vector<bool> tmpInput;
            for(unsigned int q = 0 ; q < input.second.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input.second[q]);
                }else{
                    tmpInput.push_back(input.second[q]);
                }
            }
            vector<bool> tmpOutput2 = cir2.generateOutput(tmpInput);
            outputVector2.push_back(tmpOutput2);
        }
        vector<vector<int> > stRecord1, stRecord2;
        if(!only || only == 2)change += cir1.simulationType2(output1, outputVector1, stRecord1);
        if(!only || only == 2)change += cir2.simulationType2(output2, outputVector2, stRecord2);
        dealRecord(stRecord1, stRecord2, true);
        stRecord1.clear();
        stRecord2.clear();
        if(!only || only == 3)change += cir1.simulationType3(output1, outputVector1, stRecord1);
        if(!only || only == 3)change += cir2.simulationType3(output2, outputVector2, stRecord2);
        dealRecord(stRecord1, stRecord2, false);
        if(change == 0 )noChangeNum--;
        else noChangeNum = stopNum;
    }
}

pair<vector<bool>, vector<bool>>
LargeScale::generateInput() {
    vector<bool> input1, input2, set1, set2;
    input1.resize(cir1.getInputNum());
    set1.resize(cir1.getInputNum());
    input2.resize(cir2.getInputNum());
    set2.resize(cir2.getInputNum());
    std::uniform_int_distribution<> dis(0, 1);
    for(auto cluster1 : cir1.getInputClusters()){
        for(auto cluster2 : cir2.getInputClusters()){
            if(hashTable[cluster1[0]] == hashTable[cluster2[0]]){
                bool genBool = distribution(generator);
                for(auto port : cluster1){
                    int order;
                    for(auto _order : cir1.inputIdxToOrder(cir1.getIdx(port))){
                        if(cir1.fromOrderToName(_order) == port){
                            order = _order;
                        }
                    }
                    input1[order] = genBool;
                    set1[order] = true;
                }
                for(auto port : cluster2){
                    int order;
                    for(auto _order : cir2.inputIdxToOrder(cir2.getIdx(port))){
                        if(cir2.fromOrderToName(_order) == port){
                            order = _order;
                        }
                    }
                    input2[order] = genBool;
                    set2[order] = true;
                }
                break;
            }
        }
    }
    for(unsigned int i = 0 ; i < input1.size() ; i++){
        if(!set1[i])input1[i] = distribution(generator);
    }
    for(unsigned int i = 0 ; i < input2.size() ; i++){
        if(!set2[i])input2[i] = distribution(generator);
    }
    pair<vector<bool>, vector<bool>> result{input1, input2};
    return result;
}

int LargeScale::start() {
    vector<vector<int>> initialInputRecord1, initialOutputRecord1;
    cir1.initialRefinement(initialInputRecord1, initialOutputRecord1);
    vector<vector<int> > initialInputRecord2, initialOutputRecord2;
    cir2.initialRefinement(initialInputRecord2, initialOutputRecord2);
    dealRecord(initialInputRecord1, initialInputRecord2, true);
    dealRecord(initialOutputRecord1, initialOutputRecord2, false);
    int change = 0;
    do{
        vector<vector<set<size_t> > > dependencyInputRecord1, dependencyOutputRecord1;
        vector<vector<set<size_t> > > dependencyInputRecord2, dependencyOutputRecord2;
        change = 0;
        change += cir1.dependencyAnalysis(dependencyInputRecord1, dependencyOutputRecord1, hashTable);
        change += cir2.dependencyAnalysis(dependencyInputRecord2, dependencyOutputRecord2, hashTable);
        dealRecord(dependencyInputRecord1, dependencyInputRecord2, true);
        dealRecord(dependencyOutputRecord1, dependencyOutputRecord2, false);
    } while (change != 0);
    randomSimulation();
    vector<pair<string, string>> inputMatchPair = removeNonSingleton(cir1.getInputClusters(),
                                                                     cir2.getInputClusters());
    vector<pair<string, string>> outputMatchPair = removeNonSingleton(cir1.getOutputClusters(),
                                                                      cir2.getOutputClusters());
    for(auto &port : cir1.getZero()){
        for(auto &port2 : cir2.getZero()){
            outputMatchPair.push_back(pair<string,string>{port, port2});
        }
    }
    for(auto &port : cir1.getOne()){
        for(auto &port2 : cir2.getOne()){
            outputMatchPair.push_back(pair<string,string>{port, port2});
        }
    }
    cir1.erasePort(cir1.getZero());
    cir2.erasePort(cir2.getZero());
    cir1.erasePort(cir1.getOne());
    cir2.erasePort(cir2.getOne());

//    cir1.print(hashTable);
//    cir2.print(hashTable);
    cout <<"[LargeScale]"<< cir1.getZero().size() << ' ' << cir2.getZero().size() << ' ' << cir1.getOne().size() << ' ' << cir2.getOne().size() << endl;
    cout << "First matching:" << inputMatchPair.size() << ' ' << outputMatchPair.size() <<endl;
    removeNonSupport(inputMatchPair, outputMatchPair);
    removeNonMatch(inputMatchPair, outputMatchPair);
    SAT_Solver(inputMatchPair, outputMatchPair);
    OutputStructure outputStructure;
    int matchNumber = 0;
    for(auto &match : inputMatchPair){
        Group group;
        group.cir1 = match.first;
        group.cir2.push_back(match.second);
        group.invVector.push_back(false);
        outputStructure.inputGroups.push_back(group);
    }
    for(auto &match : outputMatchPair){
        Group group;
        group.cir1 = match.first;
        group.cir2.push_back(match.second);
        group.invVector.push_back(false);
        outputStructure.outputGroups.push_back(group);
        matchNumber += 2;
    }
    cout << "Large Scale matching port number: " << matchNumber << "(" << (float)matchNumber / allOutputNumber * 100 << "%)" << endl;
    parseOutput(outputFilePath, outputStructure);
    if(matchNumber == allOutputNumber)return -1;
    return  matchNumber;
}


vector<pair<string, string>>
LargeScale::removeNonSingleton(const vector<vector<string>> &par1, const vector<vector<string>> &par2) {
    vector<pair<string, string> > result;
    vector<string> nonSingleton1, nonSingleton2;
    for(unsigned int i = 0 ; i < par1.size() ; i++){
        if(par1[i].size() == 1){
            nonSingleton1.push_back(par1[i][0]);
        }
    }
    for(unsigned int i = 0 ; i < par2.size() ; i++){
        if(par2[i].size() == 1){
            nonSingleton2.push_back(par2[i][0]);
        }
    }
    vector<vector<int> > LCS, prev;
    LCS.resize(nonSingleton1.size() + 1);
    prev.resize(nonSingleton1.size() + 1);
    for(unsigned int i = 0 ; i <= nonSingleton1.size() ; i++){
        LCS[i].resize(nonSingleton2.size() + 1);
        prev[i].resize(nonSingleton2.size() + 1);
    }
    for(unsigned int i = 1 ; i <= nonSingleton1.size() ; i++){
        for(unsigned int q = 1 ; q <= nonSingleton2.size() ; q++){
            if(hashTable[nonSingleton1[i - 1]] == hashTable[nonSingleton2[q - 1]]){
                LCS[i][q] = LCS[i - 1][q - 1] + 1;
                prev[i][q] = 0;
            }else{
                if(LCS[i - 1][q] > LCS[i][q - 1]){
                    LCS[i][q] = LCS[i - 1][q];
                    prev[i][q] = 1;
                }else{
                    LCS[i][q] = LCS[i][q - 1];
                    prev[i][q] = 2;
                }
            }
        }
    }
    unsigned int l = LCS[nonSingleton1.size()][nonSingleton2.size()], x = nonSingleton1.size(), y = nonSingleton2.size();
    while (l > 0){
        if(prev[x][y] == 0){
            result.push_back(pair<string,string> (nonSingleton1[x - 1], nonSingleton2[y - 1]));
            l--;x--;y--;
        }else if(prev[x][y] == 1){
            x--;
        }else if(prev[x][y] == 2){
            y--;
        }
    }
    return result;
}

void LargeScale::removeNonSupport(vector<pair<string, string>> &inputMatch, vector<pair<string, string>> &outputMatch) {
    int change = 0;
    set<string> inputMap1, outputMap1;
    set<string> inputMap2, outputMap2;
    for(auto &match: inputMatch){
        inputMap1.insert(match.first);
        inputMap2.insert(match.second);
    }
    for(auto &match: outputMatch){
        outputMap1.insert(match.first);
        outputMap2.insert(match.second);
    }
    do{
        change = 0;
        for(auto match = inputMatch.begin() ; match != inputMatch.end() ; ){
            bool flag1 = false, flag2 = false;
            auto supportSet1 = cir1.getSupport(match->first);
            for(auto &supportVar : supportSet1){
                if(outputMap1.find(supportVar) != outputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto &supportVar : supportSet2){
                if(outputMap2.find(supportVar) != outputMap2.end()){
                    flag2 = true;
                    break;
                }
            }
            if(!(flag1 || flag2)){
                change++;
                inputMap1.erase(match->first);
                inputMap2.erase(match->second);
                match = inputMatch.erase(match);
            }else{
                match++;
            }
        }
        for(auto match = outputMatch.begin() ; match != outputMatch.end() ; ){
            bool flag1 = false, flag2 = false;
            auto supportSet1 = cir1.getSupport(match->first);
            for(auto &supportVar : supportSet1){
                if(inputMap1.find(supportVar) == inputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto &supportVar : supportSet2){
                if(inputMap2.find(supportVar) == inputMap2.end()){
                    flag2 = true;
                    break;
                }
            }
            if(flag1 || flag2){
                change++;
                outputMap1.erase(match->first);
                outputMap2.erase(match->second);
                match = outputMatch.erase(match);

            }else{
                match++;
            }
        }
    } while (change != 0);
    return;
}

void LargeScale::SAT_Solver(vector<pair<string, string> > &inputMatch, vector<pair<string, string> > &outputMatch) {
    if(outputMatch.size() == 0)return;
    string savePath1 = "save1.aig";
    string savePath2 = "save2.aig";

    produceMatchAIG(inputMatch, outputMatch, savePath1, savePath2);
    //TODO optimize abc command
    string abcCmd = "miter " + savePath1 + " " + savePath2 + "; write_aiger -s miter.aig;";
    string resultPath = "stdoutOutput.txt";
    cout.flush();
    FILE *saveStdout = stdout;
    stdout = fopen(resultPath.c_str(), "a");
    if (stdout != NULL) {
        if (Cmd_CommandExecute(pAbc, abcCmd.c_str())){
            cout << "[LargeScale] ERROR:Cannot execute command \"" << abcCmd << "\".\n";
            exit(1);
        }
        fflush(stdout);
        fclose(stdout);
        stdout = saveStdout;
    } else {
        cout << "[LargeScale] ERROR:Can't write file:" << resultPath << endl;
        exit(1);
    }
    char miterAIG[]{"miter.aig"};
    char miterCNF[]{"miter.cnf"};
    aigtocnf(miterAIG, miterCNF);
    solverResult result = SAT_solver(miterCNF);
    if(result.satisfiable){
        // TODO Not test
        cout << "Not Implement" << endl;
        exit(1);
        AIG miter("miter.aig");
        ifstream pf("miter.cnf");
        string symbol;
        map<int, string> CNFToAIG;
        while (pf >> symbol){
            if(symbol != "c")break;
            int AIGIdx, CNFIdx;
            string tmp;
            pf >> AIGIdx >> tmp >> CNFIdx;
            //TODO deal AIGIdx == 0 or 1 and new output format
//            CNFToAIG[CNFIdx] = miter.inputFromIndexToName(AIGIdx / 2);
        }
        pf.close();
        vector<bool> inputVector1, inputVector2;
        inputVector1.resize(cir1.getInputNum());
        inputVector2.resize(cir1.getInputNum());
        for(int i = 0 ; i < result.inputSize ; i++){
            // TODO need to fit new input format
//            inputVector1[cir1.inputIdxToOrder(cir1.getIdx(CNFToAIG[i + 1]))] = (result.input[i] > 0 ? 1 : 0);
//            inputVector2[cir2.inputIdxToOrder(cir2.getIdx(CNFToAIG[i + 1]))] = (result.input[i] > 0 ? 1 : 0);
        }
        vector<bool> outputVector1 = cir1.generateOutput(inputVector1);
        vector<bool> outputVector2 = cir2.generateOutput(inputVector2);
        vector<string> removeVector1, removeVector2;
        for(unsigned int i = 0 ; i < outputVector1.size() ; i++){
            if(outputVector1[i] != outputVector2[i]){
                removeVector1.push_back(cir1.fromOrderToName(cir1.getInputNum() + i));
                removeVector2.push_back(cir2.fromOrderToName(cir2.getInputNum() + i));
            }
        }
        for(auto remove : removeVector1){
            for(auto it = outputMatch.begin() ; it != outputMatch.end();){
                if(remove == it->first){
                    outputMatch.erase(it);
                    break;
                }
            }
        }
        cir1.erasePort(removeVector1);
        cir2.erasePort(removeVector2);
        free(result.input);
        return SAT_Solver(inputMatch, outputMatch);
    }else{
        return;
    }
}

void LargeScale::produceMatchAIG(vector<pair<string, string> > inputMatch, vector<pair<string, string> > outputMatch,
                                 string savePath1,
                                 string savePath2) {
    AIG newAIG = cir2;
    for(auto &match : inputMatch){
        newAIG.changeName(match.second, match.first);
    }
    for(auto &match : outputMatch){
        newAIG.changeName(match.second, match.first);
    }
    aiger *aig = aiger_init();
    string  tmpFilePath1 = "tmp1.aig";
    string  tmpFilePath2 = "tmp2.aig";
    FILE *fp = nullptr;
    fp = fopen(tmpFilePath1.c_str(), "w+");
    fputs(newAIG.getRaw().c_str(), fp);
    fclose(fp);
    const char *err_msg = aiger_open_and_read_from_file(aig, tmpFilePath1.c_str());
    if(err_msg != nullptr){
        cout << "[LargeScale]ERROR: " << err_msg << endl;
        exit(1);
    }
    fp = fopen(savePath2.c_str(), "w");
    aiger_write_to_file(aig, aiger_binary_mode, fp);
    fclose(fp);
    aiger_reset(aig);

    aig = aiger_init();
    fp = fopen(tmpFilePath2.c_str(), "w+");
    fputs(cir1.getRaw().c_str(), fp);
    fclose(fp);
    const char *err_msg2 = aiger_open_and_read_from_file(aig, tmpFilePath2.c_str());
    if(err_msg2 != nullptr){
        cout << "[LargeScale]ERROR: " << err_msg2 << endl;
        exit(1);
    }
    fp = fopen(savePath1.c_str(), "w");
    aiger_write_to_file(aig, aiger_binary_mode, fp);
    fclose(fp);

    aiger_reset(aig);
}

void LargeScale::removeNonMatch(const vector<pair<string, string>> &inputMatch,
                                const vector<pair<string, string>> &outputMatch) {
    set<string> inputSet1, outputSet1;
    set<string> inputSet2, outputSet2;
    for(auto &matchPair : inputMatch){
        inputSet1.insert(matchPair.first);
        inputSet2.insert(matchPair.second);
    }
    for(auto &matchPair : outputMatch){
        outputSet1.insert(matchPair.first);
        outputSet2.insert(matchPair.second);
    }
    cir1.removeNonMatch(inputSet1, outputSet1);
    cir2.removeNonMatch(inputSet2, outputSet2);
}





