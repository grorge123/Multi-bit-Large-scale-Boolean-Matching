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
    vector<size_t> inputRecord;
    vector<size_t> outputRecord;
    void intialRefineCluster(vector<vector<string> > &clusters, vector<size_t> record);
    int dependencyAnalysisCluster(vector<vector<string> > &clusters, vector<vector<string> > &anotherClusters,
                                  vector<size_t> record);
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
    void initialRefinement();
    void dependencyAnalysis();
    int simulationType1(vector<bool> output);
    int simulationType2(vector<bool> originalOutput, vector<vector<bool>> outputVector);
    int simulationType3(vector<bool> originalOutput, vector<vector<bool>> outputVector);

    void print();

    template<typename T>
    size_t calculateHash(const T& variable)
    {
        std::hash<T> hasher;
        return hasher(variable);
    }
    size_t calculateHash(const set<int>& container)
    {
        size_t seed = 0;
        for (const auto& element : container) {
            seed ^= std::hash<typename set<int>::value_type>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARTITION_H
