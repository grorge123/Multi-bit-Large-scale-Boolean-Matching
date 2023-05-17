#include <gtest/gtest.h>
#include <set>
#include "../src/AIG.h"
class AIGCase : public ::testing::Test {
protected:
    void SetUp() override {
        aig = AIG("cir1.aig");
    }
    AIG aig;
};


TEST_F(AIGCase, test1) {
    set<string> a0 = aig.getSupport("a0");
    set<string> a1 = aig.getSupport("a1");
    set<string> b0 = aig.getSupport("b0");
    set<string> b1 = aig.getSupport("b1");
    set<string> c = aig.getSupport("c");
    set<string> h0 = aig.getSupport("h0");
    set<string> h1 = aig.getSupport("h1");
    set<string> m0 = aig.getSupport("m0");
    set<string> m1 = aig.getSupport("m1");
    set<string> _a0{"h0", "h1"};
    set<string> _a1{"h1"};
    set<string> _b1{"h1", "m0"};
    set<string> _b0{"h0", "h1", "m1", "m0"};
    set<string> _c{"m0", "m1"};
    set<string> _h0{"a0", "b0"};
    set<string> _h1{"a1", "b0", "b1", "a0"};
    set<string> _m0{"b1", "b0", "c"};
    set<string> _m1{"b0", "c"};
    EXPECT_EQ(a0 , _a0);
    EXPECT_EQ(a1 , _a1);
    EXPECT_EQ(b0 , _b0);
    EXPECT_EQ(b1 , _b1);
    EXPECT_EQ(c , _c);
    EXPECT_EQ(h0 , _h0);
    EXPECT_EQ(h1 , _h1);
    EXPECT_EQ(m0 , _m0);
    EXPECT_EQ(m1 , _m1);
}

TEST_F(AIGCase, test2){
    cout << "INPUT: ";
    for(int i = 0 ; i < aig.getInputNum() ; i++){
        cout << aig.fromIndexToName(i) << ' ' ;
    }
    cout << endl << "OUTPUT: ";
    for(int q = aig.getInputNum() ; q < aig.getInputNum() + aig.getOutputNum() ; q++){
        cout << aig.fromIndexToName(q) << ' ' ;
    }
    cout << endl;
    vector<bool> input1{0,1,0,1,1};
    vector<bool> input2{1,0,1,0,0};
    vector<bool> input3{1,1,1,0,1};
    vector<bool> input4{1,1,1,1,1};
    vector<bool> input5{0,0,0,0,0};
    vector<bool> output1{0,1,1,0};
    vector<bool> output2{0,1,1,0};
    vector<bool> output3{0,1,1,1};
    vector<bool> output4{0,0,0,0};
    vector<bool> output5{1,0,0,0};
    ASSERT_EQ(aig.generateOutput(input1), output1);
    ASSERT_EQ(aig.generateOutput(input2), output2);
    ASSERT_EQ(aig.generateOutput(input3), output3);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}