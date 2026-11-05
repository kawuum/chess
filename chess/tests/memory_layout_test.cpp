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
  // 12 bitboards (96 bytes) + 64 bytes squares array (since 2.3).
  EXPECT_EQ(sizeof(board), 160u);
}

TEST(MemoryLayout, Piece) {
  // bool + two 1-byte enums (12 bytes with 4-byte enums before 2.4).
  EXPECT_EQ(sizeof(piece), 3u);
}

TEST(MemoryLayout, Move) {
  // piece + 4 coordinates + 1-byte move_type + 1-byte promotion piece
  // (20 bytes before 2.4, 8 before the promotion piece became part of it).
  EXPECT_EQ(sizeof(move), 9u);
}

TEST(MemoryLayout, GameHistory) {
  // One per played move; contains a board (+64 bytes in 2.3) and a move,
  // castling rights and result (-32 bytes in 2.4). The draw reason and the
  // half move clock (4.4) fill bytes that were padding before: still 200.
  EXPECT_EQ(sizeof(game_history), 200u);
}

TEST(MemoryLayout, Engine) {
  EXPECT_EQ(sizeof(engine), 64u);  // shared_ptr + two vectors
}

}  // namespace
