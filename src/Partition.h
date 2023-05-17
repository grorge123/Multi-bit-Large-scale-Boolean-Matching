//
// Created by grorge on 5/16/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#include "AIG.h"

class Partition : AIG{

    vector<vector<string> > inputClusters;
public:
    const vector<vector<string>> &getInputClusters() const;

    const vector<vector<string>> &getOutputClusters() const;

private:
    vector<vector<string> > outputClusters;
    void intialRefineCluster(vector<vector<string> > &clusters);
public:
    Partition(string fileName) : AIG(fileName){
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
    void initialRefinement();
    void dependencyAnalysis();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
