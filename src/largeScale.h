//
// Created by Han Wen Tsao on 2023/5/19.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H

#include <string>
#include <random>
#include "Partition.h"
#include "parser.h"
using namespace std;
class LargeScale {
    string outputFilePath;
    default_random_engine generator;
    uniform_int_distribution<int> distribution;
    Partition cir1, cir2;
    int allOutputNumber;

    template<typename T>
    int compare(const T& a, const T& b){
        if(a == b){
            return 0;
        }else if(a < b){
            return -1;
        }else{
            return 1;
        }
    }
    int compare(const set<int>& set1, const set<int>& set2)
    {
        auto it1 = set1.begin();
        auto it2 = set2.begin();

        while (it1 != set1.end() && it2 != set2.end()) {
            // 在遍歷過程中，一旦發現有一對不相等的元素，就可以確定兩個集合的字典序關系
            if (*it1 != *it2) {
                return *it1 < *it2 ? -1 : 1;
            }

            // 如果兩個元素相等，則繼續遍歷下一對元素
            ++it1;
            ++it2;
        }

        // 如果所有對應元素都相等，則元素較少的集合在字典序上較小
        if (it1 == set1.end() && it2 != set2.end()) {
            return -1;  // set1 is smaller
        } else if (it1 != set1.end() && it2 == set2.end()) {
            return 1;  // set2 is smaller
        } else {
            return 0;  // both sets are equal
        }
    }
public:
    LargeScale(){

    }
    LargeScale(InputStructure input, string outputFilePath) : outputFilePath(outputFilePath), generator(7122), distribution(0, 1){
        cir1 = Partition(input.cir1AIGPath);
        cir2 = Partition(input.cir2AIGPath);
        allOutputNumber = cir2.getOutputNum() + cir1.getOutputNum();
    }
    int start();
    void produceMatchAIG(vector<pair<string, string> > inputMatch, vector<pair<string, string> > outputMatch,
                         string savePath1,
                         string savePath2);
    vector<bool> generateInput(int inputNum);
    void randomSimulation(int only = 0);
    vector<pair<string, string>> removeNonSingleton(const vector<vector<string>> &par1, const vector<vector<string>> &par2);
    void removeNonSupport(vector<pair<string, string> > &inputMatch, vector<pair<string, string> > &outputMatch);
    void SAT_Solver(vector<pair<string, string> > &inputMatch, vector<pair<string, string> > &outputMatch);

    template<typename T>
    void reduceCluster(const vector<vector<T>> &record1, const vector<vector<T>> &record2, bool isInput) {
        pair<vector<int>, vector<int> > erasePortVector = matchPartition(record1, record2);
        for(int i = erasePortVector.first.size() - 1 ; i >= 0 ; i--){
            cir1.eraseCluster(i, isInput);
        }
        for(int i = erasePortVector.second.size() - 1 ; i >= 0 ; i--){
            cir2.eraseCluster(i, isInput);
        }
    }

    template<typename T>
    pair<vector<int>, vector<int>> matchPartition(const vector<vector<T>> &record1, const vector<vector<T>> &record2) {
        pair<vector<int>, vector<int> > result;
        int accumulation1 = 0;
        int accumulation2 = 0;
        if(record1.size() != record2.size()){
            cout << "[LargeScale] ERROR: record1 size and record2 size are not equal!" << endl;
        }
        for(unsigned int idx = 0 ; idx < min(record1.size(), record2.size()) ; idx++){
            unsigned int idx1 = 0, idx2 = 0;
            while (idx1 < record1[idx].size() && idx2 < record2[idx].size()){
                int compareResult = compare(record1[idx][idx1], record2[idx][idx2]);
                if( compareResult == 0){
                    idx1++;
                    idx2++;
                }else if(compareResult == -1){
                    result.first.push_back(accumulation1 + idx1);
                    idx1++;
                }else{
                    result.second.push_back(accumulation2 + idx2);
                    idx2++;
                }
            }
            while (idx1 < record1[idx].size()){
                result.first.push_back(accumulation1 + idx1);
                idx1++;
            }
            while (idx2 < record2[idx].size()){
                result.second.push_back(accumulation2 + idx2);
                idx2++;
            }
            accumulation1 += record1[idx].size();
            accumulation2 += record2[idx].size();
        }
        return result;
    }
};

#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
