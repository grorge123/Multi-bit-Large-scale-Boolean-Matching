//
// Created by grorge on 5/16/23.
//
#include <random>
#include "Partition.h"
#include "utility.h"
void Partition::initialRefinement() {
    intialRefineCluster(inputClusters);
    intialRefineCluster(outputClusters);
}

void Partition::intialRefineCluster(vector<vector<string>> &clusters) {
    vector<vector<string> > newClusters;
    for(auto cluster : clusters){
        map<int, vector<string>> Dmap;
        for(auto port : cluster){
            Dmap[getSupport(port).size()].push_back(port);
        }
        for(auto newCluster : Dmap){
            newClusters.push_back(newCluster.second);
        }
    }
    clusters = newClusters;
    return;
}

const vector<vector<string>> &Partition::getInputClusters() const {
    return inputClusters;
}

const vector<vector<string>> &Partition::getOutputClusters() const {
    return outputClusters;
}

void Partition::dependencyAnalysis() {
    int change = 0;
    do {
        change = 0;
        change += dependencyAnalysisCluster(inputClusters, outputClusters);
        change += dependencyAnalysisCluster(outputClusters, inputClusters);
    } while (change != 0);
    return;
}

int Partition::dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<string> > &anotherClusters) {
    int change = 0;
    vector<vector<string > > newClusters;
    for(auto cluster : clusters){
        map<set<int>, vector<string> > Smap;
        for(auto port : cluster){
            set<string> supportSet = getSupport(port);
            set<int> indexSet;
            for(auto supportVar : supportSet){
                indexSet.insert( findClusterIndex(supportVar, anotherClusters));
            }
            Smap[indexSet].push_back(port);
        }
        change += Smap.size() - 1;
        for(auto newCluster : Smap){
            newClusters.push_back(newCluster.second);
        }
    }
    clusters = newClusters;
    return change;
}

int Partition::findClusterIndex(string name, vector<vector<string>> &clusters) {
    for(unsigned int i = 0 ; i < clusters.size() ; i++){
        for(auto port : clusters[i]){
            if(port == name) return i;
        }
    }
    cout << "[Partition] ERROR: Can not find port." << endl;
    return -1;
}

void Partition::print() {
    cout << "INPUT" << endl;
    cout << "START: ";
    for(auto cluster : inputClusters){
        cout << " { ";
        for(auto port: cluster){
            cout << port << ' ';
        }
        cout << " } ";
    }
    cout << endl;
    cout << "OUTPUT" << endl;
    cout << "START: ";
    for(auto cluster : outputClusters){
        cout << " { ";
        for(auto port: cluster){
            cout << port << ' ';
        }
        cout << " } ";
    }
    cout << endl;
}

void Partition::randomSimulation() {
    int stopNum = 10;
    int noChangeNum = stopNum;
    while (noChangeNum > 0){
        int change = 0;
        vector<bool> input = generateInput();
        vector<bool> output = generateOutput(input);
        vector<vector<bool>> outputVector;
        change += simulationType1(output);
        for(int i = 0 ; i < input.size() ; i++){
            vector<bool> tmpInput;
            for(int q = 0 ; q < input.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input[q]);
                }else{
                    tmpInput.push_back(input[q]);
                }
            }
            vector<bool> tmpOutput = generateOutput(tmpInput);
            change += simulationType1(tmpOutput);
            outputVector.push_back(tmpOutput);
        }
        simulationType2(input, output, outputVector);
        simulationType3(input, output, outputVector);
        if(change == 0 )noChangeNum--;
        else noChangeNum = stopNum;
    }
}

vector<bool> Partition::generateInput() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    std::vector<bool> result;
    for (int i = 0 ; i < getInputNum() ; i++) {
        result.push_back(dis(gen));
    }
    return result;
}

int Partition::simulationType1(vector<bool> output) {
    vector<vector<string> > newClusters;
    int change = 0;
    for(auto cluster : outputClusters){
        map<bool, vector<string> > Bmap;
        for(auto port : cluster){
            Bmap[output[idxToOrder(getIdx(port))]].push_back(port);
        }
        change += Bmap.size() - 1;
        for(auto newCluster : Bmap){
            newClusters.push_back(newCluster.second);
        }
    }
    outputClusters = newClusters;
    return change;
}

int Partition::simulationType2(vector<bool> input, vector<bool> originalOutput, vector<vector<bool>> outputVector) {
    vector<int> obs;
    obs.resize(getInputNum());
    for(int outputIdx = 0 ; outputIdx < outputVector.size() ; outputIdx++){
        for(int i = 0 ; i < outputVector[outputIdx].size() ; i++){
            if(outputVector[outputIdx][i] != originalOutput[i]){
                obs[outputIdx]++;
            }
        }
    }
    vector<vector<string> > newClusters;
    int change = 0;
    for(auto cluster : outputClusters){
        map<int, vector<string> > obsMap;
        for(auto port : cluster){
            obsMap[obs[idxToOrder(getIdx(port))]].push_back(port);
        }
        change += obsMap.size() - 1;
        for(auto newCluster : obsMap){
            newClusters.push_back(newCluster.second);
        }
    }
    outputClusters = newClusters;
    return change;
}

int Partition::simulationType3(vector<bool> input, vector<bool> originalOutput, vector<vector<bool>> outputVector) {
    vector<int> ctrl;
    ctrl.resize(getOutputNum());
    for(auto output : outputVector){
        for(int i = 0 ; i < output.size() ; i++){
            if(output[i] != originalOutput[i]){
                ctrl[i]++;
            }
        }
    }
    vector<vector<string> > newClusters;
    int change = 0;
    for(auto cluster : outputClusters){
        map<int, vector<string> > ctrlMap;
        for(auto port : cluster){
            ctrlMap[ctrl[idxToOrder(getIdx(port))]].push_back(port);
        }
        change += ctrlMap.size() - 1;
        for(auto newCluster : ctrlMap){
            newClusters.push_back(newCluster.second);
        }
    }
    outputClusters = newClusters;
    return change;
}
