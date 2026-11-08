// Tests for datastructures/bits.hpp: the portable bit functions (for the
// Game Boy Advance) must give the same results as the GCC builtins.
#include "../datastructures/bits.hpp"

#include <gtest/gtest.h>

#include <random>
#include <vector>

namespace {

std::vector<uint64_t> test_words() {
  std::vector<uint64_t> words = {1,
                                 2,
                                 3,
                                 0x80000000u,
                                 0x100000000u,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFF00000000u,
                                 0x8000000000000000u,
                                 0xFFFFFFFFFFFFFFFFu,
                                 0x5555555555555555u,
                                 0xAAAAAAAAAAAAAAAAu};
  for (int bit = 0; bit < 64; ++bit) words.push_back(uint64_t{1} << bit);
  std::mt19937_64 rng(2026);
  for (int i = 0; i < 10000; ++i) {
    // Also words with few bits set, as bitboards often are.
    uint64_t w = rng();
    if (i % 2) w &= rng();
    if (i % 4 == 1) w &= rng();
    if (w != 0) words.push_back(w);
  }
  return words;
}

TEST(Bits, PortablePopcountMatchesTheBuiltin) {
  EXPECT_EQ(bits::popcount_portable(0), 0);
  for (uint64_t w : test_words()) {
    ASSERT_EQ(bits::popcount_portable(w), __builtin_popcountll(w)) << w;
  }
}

TEST(Bits, PortableLeadingZerosMatchTheBuiltin) {
  for (uint64_t w : test_words()) {
    ASSERT_EQ(bits::leading_zeros_portable(w), __builtin_clzll(w)) << w;
  }
}

TEST(Bits, UsableAtCompileTime) {
  static_assert(bits::popcount_portable(0xF0F0u) == 8);
  static_assert(bits::leading_zeros_portable(1) == 63);
  static_assert(bits::popcount(0xFFu) == 8);
  static_assert(bits::leading_zeros(uint64_t{1} << 63) == 0);
}

}  // namespace
