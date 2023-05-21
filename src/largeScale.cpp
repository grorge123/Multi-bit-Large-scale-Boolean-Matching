//
// Created by Han Wen Tsao on 2023/5/19.
//

#include "largeScale.h"
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
        reduceCluster(st1Record1, st1Record2, false);
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
            reduceCluster(st1Record1, st1Record2, false);
            outputVector1.push_back(tmpOutput1);
            outputVector2.push_back(tmpOutput2);
        }
        vector<vector<int> > stRecord1, stRecord2;
        if(!only || only == 2)change += cir1.simulationType2(output1, outputVector1, stRecord1);
        if(!only || only == 2)change += cir2.simulationType2(output2, outputVector2, stRecord2);
        reduceCluster(stRecord1, stRecord2, true);

        stRecord1.clear();
        st1Record2.clear();
        if(!only || only == 3)change += cir1.simulationType3(output1, outputVector1, stRecord1);
        if(!only || only == 3)change += cir2.simulationType3(output2, outputVector2, stRecord2);
        reduceCluster(stRecord1, stRecord2, false);
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

void LargeScale::start() {
    vector<vector<int>> initialInputRecord1, initialOutputRecord1;
    cir1.initialRefinement(initialInputRecord1, initialOutputRecord1);
    vector<vector<int> > initialInputRecord2, initialOutputRecord2;
    cir2.initialRefinement(initialInputRecord2, initialOutputRecord2);
    reduceCluster(initialInputRecord1, initialInputRecord2, true);
    reduceCluster(initialOutputRecord1, initialOutputRecord2, false);

    vector<vector<set<int> > > dependencyInputRecord1, dependencyOutputRecord1;
    cir1.dependencyAnalysis(dependencyInputRecord1, dependencyOutputRecord1);
    vector<vector<set<int> > > dependencyInputRecord2, dependencyOutputRecord2;
    cir2.dependencyAnalysis(dependencyInputRecord2, dependencyOutputRecord2);
    reduceCluster(dependencyInputRecord1, dependencyInputRecord2, true);
    reduceCluster(dependencyOutputRecord1, dependencyOutputRecord2, false);

    randomSimulation();
    vector<pair<string, string>> inputMatchPair = removeNonSingleton(cir1.getInputClusters(),
                                                                     cir2.getInputClusters());
    vector<pair<string, string>> outputMatchPair = removeNonSingleton(cir1.getOutputClusters(),
                                                                      cir2.getOutputClusters());
    removeNonSupport(inputMatchPair, outputMatchPair);
    SAT_Solver(vector<pair<string, string>>(), vector<pair<string, string>>());
    //TODO output result
}


vector<pair<string, string>>
LargeScale::removeNonSingleton(const vector<vector<string>> &par1, const vector<vector<string>> &par2) {
    if(par1.size() != par2.size()){
        cout << "[LargeScale] ERROR: removeNonSingleton size are not equal!" << endl;
    }
    vector<pair<string, string> > result;
    for(int i = 0 ; i < min(par1.size(), par2.size()) ; i++){
        if(par1[i].size() == 1 && par2[i].size() == 1){
            result.push_back(pair<string,string> (par1[i][0], par2[i][0]));
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
        for(auto match = inputMatch.begin() ; match != inputMatch.end() ; match++){
            bool flag1 = false, flag2 = false;
            auto supportSet1 = cir1.getSupport(match->first);
            for(auto supportVar : supportSet1){
                if(outputMap1.find(supportVar) == outputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto supportVar : supportSet2){
                if(outputMap2.find(supportVar) == outputMap2.end()){
                    flag2 = true;
                    break;
                }
            }
            if(!(flag1 || flag2)){
                change++;
                inputMap1.erase(match->first);
                inputMap2.erase(match->second);
                inputMatch.erase(match);
            }
        }
        for(auto match = outputMatch.begin() ; match != outputMatch.end() ; match++){
            bool flag1 = false, flag2 = false;
            auto supportSet1 = cir1.getSupport(match->first);
            for(auto supportVar : supportSet1){
                if(inputMap1.find(supportVar) != inputMap1.end()){
                    flag1 = true;
                    break;
                }
            }
            auto supportSet2 = cir2.getSupport(match->second);
            for(auto supportVar : supportSet2){
                if(inputMap2.find(supportVar) != inputMap2.end()){
                    flag2 = true;
                    break;
                }
            }
            if(flag1 || flag2){
                change++;
                outputMap1.erase(match->first);
                outputMap2.erase(match->second);
                outputMatch.erase(match);
            }
        }
    } while (change != 0);
    return;
}

void LargeScale::SAT_Solver(vector<pair<string, string> > inputMatch, vector<pair<string, string> > outputMatch) {
    //TODO implement
}





