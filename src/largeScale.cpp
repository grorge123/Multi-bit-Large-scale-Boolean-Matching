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
        vector<bool> input = generateInput(max(cir1.getInputNum(), cir2.getInputNum()));
        vector<bool> output1 = cir1.generateOutput(input);
        vector<bool> output2 = cir2.generateOutput(input);
        vector<vector<bool>> outputVector1;
        vector<vector<bool>> outputVector2;
        vector<vector<bool> > st1Record1, st1Record2;
        if(!only || only == 1)change += cir1.simulationType1(output1, st1Record1);
        if(!only || only == 1)change += cir2.simulationType1(output2, st1Record2);
        for(unsigned int i = 0 ; i < input.size() ; i++){
            vector<bool> tmpInput;
            for(unsigned int q = 0 ; q < input.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input[q]);
                }else{
                    tmpInput.push_back(input[q]);
                }
            }
            vector<bool> tmpOutput1 = cir1.generateOutput(tmpInput);
            vector<bool> tmpOutput2 = cir2.generateOutput(tmpInput);
            st1Record1.clear();
            st1Record2.clear();
            if(!only || only == 1)change += cir1.simulationType1(tmpOutput1, st1Record1);
            if(!only || only == 1)change += cir2.simulationType1(tmpOutput2, st1Record2);
            dealRecord(st1Record1, st1Record2, false);
            outputVector1.push_back(tmpOutput1);
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

vector<bool> LargeScale::generateInput(int inputNum) {
    std::uniform_int_distribution<> dis(0, 1);
    std::vector<bool> result;
    for (int i = 0 ; i < inputNum ; i++) {
        result.push_back(distribution(generator));
    }
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
        vector<vector<set<int> > > dependencyInputRecord1, dependencyOutputRecord1;
        vector<vector<set<int> > > dependencyInputRecord2, dependencyOutputRecord2;
        change = 0;
        change += cir1.dependencyAnalysis(dependencyInputRecord1, dependencyOutputRecord1);
        change += cir2.dependencyAnalysis(dependencyInputRecord2, dependencyOutputRecord2);
        dealRecord(dependencyInputRecord1, dependencyInputRecord2, true);
        dealRecord(dependencyOutputRecord1, dependencyOutputRecord2, false);
    } while (change != 0);


    randomSimulation();

    vector<pair<string, string>> inputMatchPair = removeNonSingleton(cir1.getInputClusters(),
                                                                     cir2.getInputClusters());
    vector<pair<string, string>> outputMatchPair = removeNonSingleton(cir1.getOutputClusters(),
                                                                      cir2.getOutputClusters());
    //TODO removeNonSupport should be check on not to buf case
    removeNonSupport(inputMatchPair, outputMatchPair);
    SAT_Solver(inputMatchPair, outputMatchPair);
    OutputStructure outputStructure;
    int matchNumber = 0;
    for(auto match : inputMatchPair){
        Group group;
        group.cir1 = match.first;
        group.cir2.push_back(match.second);
        group.invVector.push_back(false);
        outputStructure.inputGroups.push_back(group);
    }
    for(auto match : outputMatchPair){
        Group group;
        group.cir1 = match.first;
        group.cir2.push_back(match.second);
        group.invVector.push_back(false);
        outputStructure.outputGroups.push_back(group);
        matchNumber += 2;
    }
    parseOutput(outputFilePath, outputStructure);
    if(matchNumber == allOutputNumber)return -1;
    return  matchNumber;
}


vector<pair<string, string>>
LargeScale::removeNonSingleton(const vector<vector<string>> &par1, const vector<vector<string>> &par2) {
    if(par1.size() != par2.size()){
        cout << "[LargeScale] ERROR: removeNonSingleton size are not equal!" << endl;
    }
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
    for(auto match: inputMatch){
        inputMap1.insert(match.first);
        inputMap2.insert(match.second);
    }
    for(auto match: outputMatch){
        outputMap1.insert(match.first);
        outputMap2.insert(match.second);
    }
    do{
        change = 0;
        for(auto match = inputMatch.begin() ; match != inputMatch.end() ; ){
            bool flag1 = false, flag2 = false;
            auto supportSet1 = cir1.getSupport(match->first);
            for(auto supportVar : supportSet1){
                if(outputMap1.find(supportVar) != outputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto supportVar : supportSet2){
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
            for(auto supportVar : supportSet1){
                if(inputMap1.find(supportVar) == inputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto supportVar : supportSet2){
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
    //TODO implement reduce redundancy
    string savePath1 = "save1.aig";
    string savePath2 = "save2.aig";

    produceMatchAIG(inputMatch, outputMatch, savePath1, savePath2);
    //TODO optimize abc command
    string abcCmd = "miter " + savePath1 + " " + savePath2 + "; write_aiger miter.aig;";
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
        // TODO remove non-match output and reduce redundancy
        cout << "Have not implement!" << endl;
        exit(1);
        return SAT_Solver(inputMatch, outputMatch);
    }else{
        return;
    }
}

void LargeScale::produceMatchAIG(vector<pair<string, string> > inputMatch, vector<pair<string, string> > outputMatch,
                                 string savePath1,
                                 string savePath2) {
    AIG newAIG = cir2;
    for(auto match : inputMatch){
        newAIG.changeName(match.second, match.first);
    }
    for(auto match : outputMatch){
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





