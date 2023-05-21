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
        if(!only || only == 1)change += cir1.simulationType1(output1);
        if(!only || only == 1)change += cir2.simulationType1(output2);
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
            if(!only || only == 1)change += cir1.simulationType1(tmpOutput1);
            if(!only || only == 1)change += cir2.simulationType1(tmpOutput2);
            outputVector1.push_back(tmpOutput1);
            outputVector2.push_back(tmpOutput2);
        }
        if(!only || only == 2)change += cir1.simulationType2(output1, outputVector1);
        if(!only || only == 2)change += cir2.simulationType2(output2, outputVector2);
        if(!only || only == 3)change += cir1.simulationType3(output1, outputVector1);
        if(!only || only == 3)change += cir2.simulationType3(output2, outputVector2);
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
    cir1.initialRefinement();
    cir2.initialRefinement();
    cir1.dependencyAnalysis();
    cir2.dependencyAnalysis();
    randomSimulation();
    //TODO SAT solver
}
