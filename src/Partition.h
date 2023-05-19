//
// Created by grorge on 5/16/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#include "AIG.h"
#include <random>
class Partition : public AIG {
    default_random_engine generator;
    uniform_int_distribution<int> distribution;
    vector<vector<string> > inputClusters;
    vector<vector<string> > outputClusters;
    void intialRefineCluster(vector<vector<string> > &clusters);
    int dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<string> > &anotherClusters);
    int findClusterIndex(string name, vector<vector<string> > &clusters);
    int simulationType1(vector<bool> output);
    int simulationType2(vector<bool> originalOutput, vector<vector<bool>> outputVector);
    int simulationType3(vector<bool> originalOutput, vector<vector<bool>> outputVector);
public:
    Partition(){};
    Partition(string fileName) : AIG(fileName), generator(7122), distribution(0,1) {
        vector<string> ve;
        for(int i = 0 ; i < getInputNum() ; i++){
            ve.push_back(fromIndexToName(i));
        }
        inputClusters.push_back(ve);
        ve.clear();
        for(int i = getInputNum() ; i < getInputNum() + getOutputNum() ; i++){
            ve.push_back(fromIndexToName(i));
        }
        outputClusters.push_back(ve);
    }
    const vector<vector<string>> &getInputClusters() const;
    const vector<vector<string>> &getOutputClusters() const;
    vector<bool> generateInput();
    void initialRefinement();
    void dependencyAnalysis();
    void randomSimulation(int only = 0);
    void print();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
