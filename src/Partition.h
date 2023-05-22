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
    int dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<string> > &anotherClusters,
                                  vector<vector<set<int>>> &record);
    int findClusterIndex(string name, vector<vector<string> > &clusters);
public:
    Partition(){};
    Partition(string fileName) : AIG(fileName) {
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
    void initialRefinement(vector<vector<int> > &inputRecord, vector<vector<int> > &outputRecord);
    int dependencyAnalysis(vector<vector<set<int> > > &inputRecord, vector<vector<set<int> > > &outputRecord);
    int simulationType1(vector<bool> output, vector<vector<bool> > &record);
    int simulationType2(vector<bool> originalOutput, vector<vector<bool>> outputVector, vector<vector<int> > &record);
    int simulationType3(vector<bool> originalOutput, vector<vector<bool>> outputVector, vector<vector<int> > &record);
    void removeNonMatch(const set<string> & inputMatch, const set<string> & outputMatch);

    void print();

};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
