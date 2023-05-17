//
// Created by grorge on 5/16/23.
//

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
