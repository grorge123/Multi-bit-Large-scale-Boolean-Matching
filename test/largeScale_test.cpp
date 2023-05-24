//
// Created by grorge on 5/17/23.
//
#include <gtest/gtest.h>
#include <set>
#include "../src/largeScale.h"
class largeCase : public ::testing::Test {
protected:
    void SetUp() override {
        cir1 = Partition("cir2.aig");
        cir2 = Partition("cir3.aig");
        lc = LargeScale(cir1, cir2);
    }
    Partition cir1;
    Partition cir2;
    LargeScale lc;
};

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}