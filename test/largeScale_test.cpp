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



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}