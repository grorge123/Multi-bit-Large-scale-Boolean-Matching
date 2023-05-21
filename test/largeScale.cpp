//
// Created by grorge on 5/17/23.
//
#include <gtest/gtest.h>
#include <set>
#include "../src/largeScale.h"
class largeCase : public ::testing::Test {
protected:
    void SetUp() override {
        lc = LargeScale();
    }
    LargeScale lc;
};

TEST_F(largeCase, Test1) {
    vector<vector<int> > initialRecord1{{1,4},{1,2,4}}, initialRecord2{{1},{1,3,4}};
    auto result = lc.matchPartition(initialRecord1, initialRecord2);
    vector<int> ans1{1, 3}, ans2{2};
    ASSERT_EQ(result.first, ans1);
    ASSERT_EQ(result.second, ans2);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}