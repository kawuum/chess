// Sizes of the core data structures. These numbers are not a goal, they
// document the current state: if a change to board, piece, move or
// game_history changes a size, this test fails and the new size has to be
// entered here on purpose. So a bigger (or smaller) board never goes unnoticed.
// The memory use at runtime is measured by bench/chess_mem.cpp.
#include <gtest/gtest.h>

#include "../engine/engine.hpp"

namespace {

TEST(MemoryLayout, BitMatrix) {
  EXPECT_EQ(sizeof(bit_matrix), 8u);  // one uint64_t
}

TEST(MemoryLayout, Board) {
  EXPECT_EQ(sizeof(board), 96u);  // 12 bitboards
}

TEST(MemoryLayout, Piece) {
  // bool + two 4-byte enums. 2 bytes would be enough (plan, phase 2).
  EXPECT_EQ(sizeof(piece), 12u);
}

TEST(MemoryLayout, Move) {
  EXPECT_EQ(sizeof(move), 20u);  // piece + 4 coordinates + 4-byte move_type
}

TEST(MemoryLayout, GameHistory) {
  EXPECT_EQ(sizeof(game_history), 168u);  // one per played move
}

TEST(MemoryLayout, Engine) {
  EXPECT_EQ(sizeof(engine), 64u);  // shared_ptr + two vectors
}

}  // namespace
