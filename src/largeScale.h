//
// Created by Han Wen Tsao on 2023/5/19.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H

#include <string>
#include <random>
#include "utility.h"
#include "Partition.h"
#include "parser.h"
using namespace std;
class LargeScale {
    string outputFilePath;
    default_random_engine generator;
    uniform_int_distribution<int> distribution;
    Partition cir1, cir2;
    int allOutputNumber;
    map<string, size_t> hashTable;

    template<class _T>
    std::size_t computeHash(const _T& input) {
        std::hash<int> hasher;
        return hasher(input);
    }

    std::size_t computeHash(const std::set<int>& s) {
        std::size_t seed = s.size();
        for(auto& i : s) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    std::size_t combineHashes(std::size_t h1, std::size_t h2) {
        // Inspired by boost's hash_combine
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
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
    void dealRecord(const vector<vector<T>> &record1, const vector<vector<T>> &record2, bool isInput){
        if(isInput){
            calculateNewHash(record1, cir1.getInputClusters());
            calculateNewHash(record2, cir2.getInputClusters());
        }else{
            calculateNewHash(record1, cir1.getOutputClusters());
            calculateNewHash(record2, cir2.getOutputClusters());
        }
    }
    template<typename T>
    void calculateNewHash(const vector<vector<T>> &record, const vector<vector<string> > &partition) {
        int idx = 0;
        for(auto clusters : record){
            for(auto cluster : clusters ){
                for(auto port: partition[idx]){
                    hashTable[port] = combineHashes(hashTable[port], computeHash(cluster));
                }
                idx++;
            }
        }
    }
    void removeNonMatch(const vector<pair<string, string> > & inputMatch, const vector<pair<string, string> > & outputMatch);
};

#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_LARGESCALE_H
