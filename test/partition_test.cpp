//
// Created by grorge on 5/17/23.
//
#include <gtest/gtest.h>
#include <set>
#include "../src/Partition.h"
class ParCase : public ::testing::Test {
protected:
    void SetUp() override {
        pa = Partition("cir1.aig");
    }
    Partition pa;
};
default_random_engine generator(7122);
uniform_int_distribution<int> distribution(0, 1);
vector<bool> generateInput(int inputNum) {
    std::uniform_int_distribution<> dis(0, 1);
    std::vector<bool> result;
    for (int i = 0 ; i < inputNum ; i++) {
        result.push_back(distribution(generator));
    }
    return result;
}
TEST_F(ParCase, Test1) {
    vector<vector<int>> a, b;
    pa.initialRefinement(a, b);
    vector<vector<string> > input{{"a1"},{"a0", "b1", "c"},{"b0"}};
    vector<vector<string> > output{{"h0","m1"},{"m0"},{"h1"}};
    ASSERT_EQ(pa.getInputClusters(), input);
    ASSERT_EQ(pa.getOutputClusters(), output);
}

TEST_F(ParCase, Test2){
    vector<vector<int>> a, b;
    pa.initialRefinement(a, b);
    vector<vector<set<size_t> > > c, d;
    map<string,size_t> hashMap{{"a1",0},{"a0",1},{"b1",1},{"c",1},{"b0",2},{"h0",0},{"m1",0},{"m0",1},{"h1",2}};
    pa.dependencyAnalysis(c, d, hashMap);
    vector<vector<string> > input{{"a1"},{"c"}, {"a0"}, {"b1"}, {"b0"}};
    vector<vector<string> > output{{"h0","m1"},{"m0"},{"h1"}};
    ASSERT_EQ(input, pa.getInputClusters());
    ASSERT_EQ(output, pa.getOutputClusters());
}

TEST_F(ParCase, TestSim1){
    int stopNum = 10;
    int noChangeNum = stopNum;
    while (noChangeNum > 0){
        int change = 0;
        vector<bool> input = generateInput(pa.getInputNum());
        vector<bool> output = pa.generateOutput(input);
        vector<vector<bool>> outputVector;
        vector<vector<bool> > a;
        change += pa.simulationType1(output, a);
        if(change == 0 )noChangeNum--;
        else noChangeNum = stopNum;
    }
    ASSERT_EQ(pa.getOutputClusters().size(), pa.getOutputNum());
}

TEST_F(ParCase, TestSim2){
    int stopNum = 10;
    int noChangeNum = stopNum;
    while (noChangeNum > 0){
        int change = 0;
        vector<bool> input = generateInput(pa.getInputNum());
        vector<bool> output = pa.generateOutput(input);
        vector<vector<bool>> outputVector;
        for(unsigned int i = 0 ; i < input.size() ; i++){
            vector<bool> tmpInput;
            for(unsigned int q = 0 ; q < input.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input[q]);
                }else{
                    tmpInput.push_back(input[q]);
                }
            }
            vector<bool> tmpOutput = pa.generateOutput(tmpInput);
            outputVector.push_back(tmpOutput);
        }
        vector<vector<int> > a;
        change += pa.simulationType2(output, outputVector, a);
        if(change == 0 )noChangeNum--;
        else noChangeNum = stopNum;
    }
    ASSERT_EQ(pa.getInputClusters().size(), pa.getInputNum());
}

TEST_F(ParCase, TestSim3){
    int stopNum = 10;
    int noChangeNum = stopNum;
    while (noChangeNum > 0){
        int change = 0;
        vector<bool> input = generateInput(pa.getInputNum());
        vector<bool> output = pa.generateOutput(input);
        vector<vector<bool>> outputVector;
        for(unsigned int i = 0 ; i < input.size() ; i++){
            vector<bool> tmpInput;
            for(unsigned int q = 0 ; q < input.size() ; q++){
                if(i == q){
                    tmpInput.push_back(!input[q]);
                }else{
                    tmpInput.push_back(input[q]);
                }
            }
            vector<bool> tmpOutput = pa.generateOutput(tmpInput);
            outputVector.push_back(tmpOutput);
        }
        vector<vector<int> > a;
        change += pa.simulationType3(output, outputVector, a);
        if(change == 0 )noChangeNum--;
        else noChangeNum = stopNum;
    }
    ASSERT_EQ(pa.getOutputClusters().size(), pa.getOutputNum());
}

//TEST_F(ParCase, Test4) {
//
//}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}