//
// Created by grorge on 5/16/23.
//

#include "Partition.h"

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
}

const vector<vector<string>> &Partition::getInputClusters() const {
    return inputClusters;
}

const vector<vector<string>> &Partition::getOutputClusters() const {
    return outputClusters;
}
