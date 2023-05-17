//
// Created by grorge on 5/17/23.
//
#include <gtest/gtest.h>
#include <set>
#include "../src/Partition.h"
Partition pa("cir1.aig");


TEST(testCase, test1) {
    pa.initialRefinement();
    vector<vector<string> > input{{"a1"},{"a0", "b1", "c"},{"b0"}};
    vector<vector<string> > output{{"h0","m1"},{"m0"},{"h1"}};
    ASSERT_EQ(pa.getInputClusters(), input);
    ASSERT_EQ(pa.getOutputClusters(), output);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}