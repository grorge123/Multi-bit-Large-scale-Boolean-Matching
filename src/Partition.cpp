//
// Created by grorge on 5/16/23.
//
#include <random>
#include "Partition.h"
#include "utility.h"
void Partition::initialRefinement(vector<vector<int> > &inputRecord, vector<vector<int> > &outputRecord) {
    intialRefineCluster(inputClusters, inputRecord);
    intialRefineCluster(outputClusters, outputRecord);
}

void Partition::intialRefineCluster(vector<vector<string> > &clusters, vector<vector<int>> &record) {
    vector<vector<string> > newClusters;
    for(auto cluster : clusters){
        map<int, vector<string>> Dmap;
        for(auto port : cluster){
            Dmap[getSupport(port).size()].push_back(port);
        }
        vector<int> recordVector;
        for(auto newCluster : Dmap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
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

void Partition::dependencyAnalysis(vector<vector<set<int> > > &inputRecord, vector<vector<set<int> > > &outputRecord) {
    int change = 0;
    do {
        change = 0;
        change += dependencyAnalysisCluster(inputClusters, outputClusters, inputRecord);
        change += dependencyAnalysisCluster(outputClusters, inputClusters, outputRecord);
    } while (change != 0);
    return;
}

int Partition::dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<string> > &anotherClusters,
                                         vector<vector<set<int>>> &record) {
    int change = 0;
    vector<vector<string > > newClusters;
    for(auto cluster : clusters){
        map<set<int>, vector<string> > Smap;
        for(auto port : cluster){
            set<string> supportSet = getSupport(port);
            set<int> indexSet;
            for(auto supportVar : supportSet){
                indexSet.insert(findClusterIndex(supportVar, anotherClusters));
            }
            Smap[indexSet].push_back(port);
        }
        change += Smap.size() - 1;
        vector<set<int> > recordVector;
        for(auto newCluster : Smap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
    }
    clusters = newClusters;
    return change;
}

int Partition::findClusterIndex(string name, vector<vector<string> > &clusters) {
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



int Partition::simulationType1(vector<bool> output, vector<vector<bool> > &record) {
    vector<vector<string> > newClusters;
    int change = 0;
    for(auto cluster : outputClusters){
        map<bool, vector<string> > Bmap;
        for(auto port : cluster){
            Bmap[output[idxToOrder(getIdx(port)) - getInputNum()]].push_back(port);
        }
        change += Bmap.size() - 1;
        vector<bool> recordVector;
        for(auto newCluster : Bmap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
    }
    outputClusters = newClusters;
    return change;
}

int Partition::simulationType2(vector<bool> originalOutput, vector<vector<bool>> outputVector,
                               vector<vector<int> > &record) {
    vector<int> obs;
    obs.resize(getInputNum());
    for(unsigned int outputIdx = 0 ; outputIdx < outputVector.size() ; outputIdx++){
        for(unsigned int i = 0 ; i < outputVector[outputIdx].size() ; i++){
            if(outputVector[outputIdx][i] != originalOutput[i]){
                obs[outputIdx]++;
            }
        }
    }
    vector<vector<string> > newClusters;
    int change = 0;
    for(auto cluster : inputClusters){
        map<int, vector<string> > obsMap;
        for(auto port : cluster){
            obsMap[obs[idxToOrder(getIdx(port))]].push_back(port);
        }
        change += obsMap.size() - 1;
        vector<int> recordVector;
        for(auto newCluster : obsMap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
    }
    inputClusters = newClusters;
    return change;
}

int Partition::simulationType3(vector<bool> originalOutput, vector<vector<bool>> outputVector,
                               vector<vector<int> > &record) {
    vector<int> ctrl;
    ctrl.resize(getOutputNum());
    for(auto output : outputVector){
        for(unsigned int i = 0 ; i < output.size() ; i++){
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
            ctrlMap[ctrl[idxToOrder(getIdx(port)) - getInputNum()]].push_back(port);
        }
        change += ctrlMap.size() - 1;
        vector<int> recordVector;
        for(auto newCluster : ctrlMap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
    }
    outputClusters = newClusters;
    return change;
}

void Partition::eraseCluster(int clusterIdx, bool isInput) {
    if(isInput){
        //TODO finish eraseSupportPort
//        for(auto port : inputClusters[clusterIdx]){
//            eraseSupportPort(port);
//        }
        inputClusters.erase(inputClusters.begin() + clusterIdx);
    }else{
        outputClusters.erase(outputClusters.begin() + clusterIdx);
    }
}


