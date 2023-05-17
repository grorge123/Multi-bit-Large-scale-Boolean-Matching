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

TEST_F(ParCase, Test1) {
    pa.initialRefinement();
    vector<vector<string> > input{{"a1"},{"a0", "b1", "c"},{"b0"}};
    vector<vector<string> > output{{"h0","m1"},{"m0"},{"h1"}};
    ASSERT_EQ(pa.getInputClusters(), input);
    ASSERT_EQ(pa.getOutputClusters(), output);
}

TEST_F(ParCase, Test2){
    pa.initialRefinement();
    pa.dependencyAnalysis();
    vector<vector<string> > input{{"a1"},{"c"}, {"a0"}, {"b1"}, {"b0"}};
    vector<vector<string> > output{{"m1"},{"h0"},{"m0"},{"h1"}};
    ASSERT_EQ(input, pa.getInputClusters());
    ASSERT_EQ(output, pa.getOutputClusters());
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}