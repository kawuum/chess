// Tests for bit_matrix, the 8x8 bitboard behind board. They also pin down the
// bit layout: square (x, y) is bit number 63 - (8 * y + x), so a1 is the
// highest bit and h8 the lowest.
#include "../datastructures/bit_matrix.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

constexpr uint64_t kTopBit = uint64_t{1} << 63;

TEST(BitMatrix, StartsEmpty) {
  bit_matrix b;
  EXPECT_EQ(b.get_raw(), 0u);
  EXPECT_EQ(b.get_num_bits_set(), 0);
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) EXPECT_FALSE(b.get_bit_at(x, y));
  }
}

TEST(BitMatrix, BitLayout) {
  bit_matrix a1, h1, a2, h8, e4;
  a1.set_bit_at(0, 0);
  h1.set_bit_at(7, 0);
  a2.set_bit_at(0, 1);
  h8.set_bit_at(7, 7);
  e4.set_bit_at(4, 3);
  EXPECT_EQ(a1.get_raw(), kTopBit);
  EXPECT_EQ(h1.get_raw(), kTopBit >> 7);
  EXPECT_EQ(a2.get_raw(), kTopBit >> 8);
  EXPECT_EQ(h8.get_raw(), uint64_t{1});
  EXPECT_EQ(e4.get_raw(), kTopBit >> (8 * 3 + 4));
}

TEST(BitMatrix, SetGetUnsetEverySquare) {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      bit_matrix b;
      b.set_bit_at(x, y);
      EXPECT_TRUE(b.get_bit_at(x, y));
      EXPECT_EQ(b.get_num_bits_set(), 1);
      b.unset_bit_at(x, y);
      EXPECT_EQ(b.get_raw(), 0u);
    }
  }
}

TEST(BitMatrix, SetAndUnsetAreIdempotent) {
  bit_matrix b;
  b.set_bit_at(3, 3);
  b.set_bit_at(3, 3);
  EXPECT_EQ(b.get_num_bits_set(), 1);
  b.unset_bit_at(5, 5);  // was not set
  EXPECT_EQ(b.get_num_bits_set(), 1);
  b.unset_bit_at(3, 3);
  b.unset_bit_at(3, 3);
  EXPECT_EQ(b.get_num_bits_set(), 0);
}

TEST(BitMatrix, OtherBitsStayUntouched) {
  bit_matrix b;
  b.set_bit_at(0, 0);
  b.set_bit_at(7, 7);
  b.set_bit_at(4, 4);
  b.unset_bit_at(4, 4);
  EXPECT_TRUE(b.get_bit_at(0, 0));
  EXPECT_TRUE(b.get_bit_at(7, 7));
  EXPECT_FALSE(b.get_bit_at(4, 4));
  EXPECT_EQ(b.get_num_bits_set(), 2);
}

TEST(BitMatrix, CountsAllBits) {
  bit_matrix full(~uint64_t{0});
  EXPECT_EQ(full.get_num_bits_set(), 64);
  full.clear();
  EXPECT_EQ(full.get_num_bits_set(), 0);
}

TEST(BitMatrix, Operators) {
  bit_matrix a(0b1100), b(0b1010);
  EXPECT_EQ((a & b).get_raw(), 0b1000u);
  EXPECT_EQ((a | b).get_raw(), 0b1110u);
  EXPECT_TRUE(a == bit_matrix(0b1100));
  EXPECT_TRUE(a != b);
  EXPECT_TRUE((a & b) != 0);
  EXPECT_TRUE((a & bit_matrix(0b0011)) == 0);
}

}  // namespace
