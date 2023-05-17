#include <gtest/gtest.h>
#include <set>
#include <bitset>
#include "../src/AIG.h"
vector<bool> top(bool a0, bool a1, bool b1, bool b0, bool c) {
    bool na, a, b, ne, e, d, na0, nb1, nb0;
    bool h0, h1, m0, m1;

    a = a1 && b0;
    na = !a;
    na0 = !a0;
    nb1 = !b1;
    b = na0 || nb1;
    e = b0 && c;
    ne = !e;
    d = ne ^ b1;

    h0 = a0 && b0;
    h1 = na ^ b;
    m0 = !d;
    m1 = b0 ^ c;

    return vector<bool>{h0, h1, m0, m1};
}
std::vector<bool> intToBoolVector(int n) {
    std::bitset<5> b(n);
    std::vector<bool> boolVec;

    for(int i = 0; i < 5; ++i) {
        boolVec.push_back(b[i]);
    }

    return boolVec;
}
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
    for(int i = 0; i < 32; ++i) {
        vector<bool> input = intToBoolVector(i);
        vector<bool> output = top(input[0], input[1], input[2], input[3], input[4]);
        ASSERT_EQ(aig.generateOutput(input), output);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}