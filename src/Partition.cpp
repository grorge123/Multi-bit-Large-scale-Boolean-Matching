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

int Partition::dependencyAnalysis(vector<vector<set<size_t>>> &inputRecord, vector<vector<set<size_t>>> &outputRecord,
                                  map<string, size_t> &hashTable) {
    int change = 0;
    change += dependencyAnalysisCluster(inputClusters, inputRecord, hashTable);
    change += dependencyAnalysisCluster(outputClusters, outputRecord, hashTable);
    return change;
}

int Partition::dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<set<size_t>>> &record,
                                         map<string, size_t> &hashTable) {
    int change = 0;
    vector<vector<string > > newClusters;
    for(auto cluster : clusters){
        map<set<size_t>, vector<string> > Smap;
        for(auto port : cluster){
            set<string> supportSet = getSupport(port);
            set<size_t> indexSet;
            for(auto supportVar : supportSet){
                indexSet.insert(hashTable[supportVar]);
            }
            Smap[indexSet].push_back(port);
        }
        change += Smap.size() - 1;
        vector<set<size_t> > recordVector;
        for(auto newCluster : Smap){
            newClusters.push_back(newCluster.second);
            recordVector.push_back(newCluster.first);
        }
        record.push_back(recordVector);
    }
    clusters = newClusters;
    return change;
}


void Partition::print() {
    cout << "INPUT: { ";
    for(auto cluster : inputClusters){
        cout << " { ";
        for(auto port: cluster){
            cout << port << ", ";
        }
        cout << " } ";
    }
    cout << " }" << endl;
    cout << "OUTPUT: { ";
    for(auto cluster : outputClusters){
        cout << " { ";
        for(auto port: cluster){
            cout << port << ", ";
        }
        cout << " } ";
    }
    cout <<" }"<< endl;
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

void Partition::removeNonMatch(const set<string> &inputMatch, const set<string> & outputMatch) {
    vector<string> removeVector;
    auto getPort = [&](set<string> match, vector<vector<string> > par) {
        for (auto clusters = par.begin(); clusters != par.end();) {
            for (auto cluster = clusters->begin(); cluster != clusters->end();) {
                if (match.find(*cluster) == match.end()) {
                    removeVector.push_back(*cluster);
                    cluster = clusters->erase(cluster);
                } else {
                    cluster++;
                }
            }
            if (clusters->size() == 0) {
                clusters = par.erase(clusters);
            } else {
                clusters++;
            }
        }
    };
    getPort(inputMatch, inputClusters);
    getPort(outputMatch, outputClusters);
    erasePort(removeVector);
}




