//
// Created by grorge on 5/16/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
#include "AIG.h"
#include <random>
class Partition : public AIG {

    vector<vector<string> > inputClusters;
    vector<vector<string> > outputClusters;
    void intialRefineCluster(vector<vector<string> > &clusters, vector<vector<int>> &record);
    int dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<set<size_t>>> &record,
                                  map<string, size_t> &hashTable);
public:
    Partition(){};
    Partition(string fileName, string cirName="") : AIG(fileName, cirName) {
        vector<string> ve;
        for(int i = 0 ; i < getInputNum() ; i++){
            if(fromOrderToIndex(i) != 0)
                ve.push_back(fromOrderToName(i));
        }
        inputClusters.push_back(ve);
        ve.clear();
        for(int i = getInputNum() ; i < getInputNum() + getOutputNum() ; i++){
            if(fromOrderToIndex(i) != 0 && !isInput(fromOrderToIndex(i)))
                ve.push_back(fromOrderToName(i));
        }
        outputClusters.push_back(ve);
    }
    const vector<vector<string>> &getInputClusters() const;
    const vector<vector<string>> &getOutputClusters() const;
    void initialRefinement(vector<vector<int> > &inputRecord, vector<vector<int> > &outputRecord);
    int dependencyAnalysis(vector<vector<set<size_t>>> &inputRecord, vector<vector<set<size_t>>> &outputRecord,
                           map<string, size_t> &hashTable);
    int simulationType1(vector<bool> output, vector<vector<bool> > &record);
    int simulationType2(vector<bool> originalOutput, vector<vector<bool>> outputVector, vector<vector<int> > &record);
    int simulationType3(vector<bool> originalOutput, vector<vector<bool>> outputVector, vector<vector<int> > &record);
    void removeNonMatch(const set<string> & inputMatch, const set<string> & outputMatch);

    void print(map<string, size_t> hashTable = map<string,size_t>{});

};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
