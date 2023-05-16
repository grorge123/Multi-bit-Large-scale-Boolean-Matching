#include <gtest/gtest.h>
#include <unordered_set>
#include "../src/AIG.h"
AIG aig("cir1.aig");
template<typename T>
void hash_combine(std::size_t& seed, const T& value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// 计算 vector 的哈希值（不考虑顺序）
template<typename T>
std::size_t vector_hash(const std::vector<T>& vec) {
    std::size_t seed = 0;

    std::unordered_set<T> uniqueElements(vec.begin(), vec.end());

    for (const auto& element : uniqueElements) {
        hash_combine(seed, element);
    }

    return seed;
}

TEST(testCase, test1) {
    vector<string> a0 = aig.getSupport("a0");
    EXPECT_EQ(vector_hash(a0) , vector_hash(vector<string>{"h0", "h1"}));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}