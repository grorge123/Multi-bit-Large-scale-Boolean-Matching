//
// Created by grorge on 6/25/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H

#include "Partition.h"
#include "parser.h"
#include "AIG.h"
class TwoStep {
    typedef pair<string, string> MP;
    string outputFilePath;
    Partition cir1, cir2;
    int allOutputNumber;

    // hyper parameter
    int maxRunTime; // ms


    template <typename T>
    struct VectorHash {
        template <typename U = T>
        std::size_t calHash(const U& a,
                            typename std::enable_if<!std::is_same<U, std::pair<std::string, std::string>>::value>::type* = nullptr) const {
            return std::hash<U>{}(a);
        }
        size_t calHash(pair<string, string> p) const {
            std::size_t hash = std::hash<std::string>{}(p.first);
            hash ^= std::hash<std::string>{}(p.second) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
        size_t doHash(const vector<T>& vec) const {
            size_t seed = vec.size();
            for (const auto& elem : vec) {
                seed ^= calHash(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
    static int nowMs();

public:
    TwoStep(){

    }
    TwoStep(Partition cir1, Partition cir2): cir1(cir1), cir2(cir2){

    }
    TwoStep(InputStructure input, string outputFilePath) : outputFilePath(outputFilePath){
        cir1 = Partition(input.cir1AIGPath, "!", false);
        cir2 = Partition(input.cir2AIGPath, "@", false);
        allOutputNumber = (cir2.getOutputNum() + cir1.getOutputNum());
    }
    void start();
    vector<MP> outputSolver();
    bool inputSolver();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_TWOSTEP_H
