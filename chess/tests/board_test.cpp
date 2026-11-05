// Tests for board: adding, moving and removing pieces, including the order of
// operations the engine uses for captures and promotions, and a consistency
// check (check_sanity) over many random games.
#include "../datastructures/board.hpp"

#include <gtest/gtest.h>

#include "../ai/random/random_ai.hpp"
#include "test_helpers.hpp"

namespace {

const piece kWhiteRook(ROOK, piece_color::WHITE);
const piece kBlackKnight(KNIGHT, piece_color::BLACK);

void expect_piece(board& b, int x, int y, piece_type t, piece_color c) {
  piece p = b.get_piece(x, y);
  ASSERT_TRUE(p.is_valid()) << "square " << x << "," << y << " is empty";
  EXPECT_EQ(p.get_piece_type(), t);
  EXPECT_EQ(p.get_piece_color(), c);
}

TEST(Board, StartsEmpty) {
  board b;
  EXPECT_TRUE(b.is_empty());
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) EXPECT_FALSE(b.get_piece(x, y).is_valid());
  }
}

TEST(Board, AddGetRemove) {
  board b;
  b.add_piece(kWhiteRook, 0, 0);
  expect_piece(b, 0, 0, ROOK, piece_color::WHITE);
  EXPECT_EQ(b.get_num_pieces(), 1);
  b.remove_piece(kWhiteRook, 0, 0);
  EXPECT_FALSE(b.get_piece(0, 0).is_valid());
  EXPECT_TRUE(b.is_empty());
}

TEST(Board, RemoveBySquare) {
  board b;
  b.add_piece(kBlackKnight, 3, 4);
  b.remove_piece(3, 4);
  EXPECT_FALSE(b.get_piece(3, 4).is_valid());
  EXPECT_TRUE(b.is_empty());
}

TEST(Board, MovePiece) {
  board b;
  b.add_piece(kWhiteRook, 0, 0);
  b.move_piece(kWhiteRook, 0, 0, 0, 5);
  EXPECT_FALSE(b.get_piece(0, 0).is_valid());
  expect_piece(b, 0, 5, ROOK, piece_color::WHITE);
  b.move_piece(0, 5, 7, 5);  // the overload that looks the piece up
  expect_piece(b, 7, 5, ROOK, piece_color::WHITE);
  EXPECT_EQ(b.get_num_pieces(), 1);
}

TEST(Board, MoveWithCapture) {
  board b;
  b.add_piece(kWhiteRook, 0, 0);
  b.add_piece(kBlackKnight, 0, 7);
  b.move_piece(kWhiteRook, 0, 0, kBlackKnight, 0, 7);
  expect_piece(b, 0, 7, ROOK, piece_color::WHITE);
  EXPECT_EQ(b.get_num_pieces(), 1);
  EXPECT_TRUE(b.check_sanity());
}

TEST(Board, CaptureInTheEnginesOrder) {
  // engine::apply_move first moves the own piece onto the target square and
  // only then removes the captured piece from it. The moved piece must stay.
  board b;
  b.add_piece(kWhiteRook, 0, 0);
  b.add_piece(kBlackKnight, 0, 7);
  b.move_piece(kWhiteRook, 0, 0, 0, 7);
  b.remove_piece(kBlackKnight, 0, 7);
  expect_piece(b, 0, 7, ROOK, piece_color::WHITE);
  EXPECT_EQ(b.get_num_pieces(), 1);
  EXPECT_TRUE(b.check_sanity());
}

TEST(Board, PromotionInTheEnginesOrder) {
  // Pawn moves to the last rank, is removed there and replaced by a queen;
  // with a capture the captured piece is removed last.
  board b;
  piece pawn(PAWN, piece_color::WHITE), queen(QUEEN, piece_color::WHITE);
  b.add_piece(pawn, 0, 6);
  b.add_piece(kBlackKnight, 1, 7);
  b.move_piece(pawn, 0, 6, 1, 7);
  b.remove_piece(pawn, 1, 7);
  b.add_piece(queen, 1, 7);
  b.remove_piece(kBlackKnight, 1, 7);
  expect_piece(b, 1, 7, QUEEN, piece_color::WHITE);
  EXPECT_EQ(b.get_num_pieces(), 1);
  EXPECT_TRUE(b.check_sanity());
}

TEST(Board, RemovingAnotherPieceTypeKeepsThePiece) {
  board b;
  b.add_piece(kWhiteRook, 2, 2);
  b.remove_piece(kBlackKnight, 2, 2);  // there is no knight
  expect_piece(b, 2, 2, ROOK, piece_color::WHITE);
}

TEST(Board, ClearEmptiesEverything) {
  engine e;
  e.new_game();
  board b = e.get_current_board();
  b.clear();
  EXPECT_TRUE(b.is_empty());
  EXPECT_FALSE(b.get_piece(4, 0).is_valid());
}

TEST(Board, CopiesAreIndependent) {
  board a;
  a.add_piece(kWhiteRook, 0, 0);
  board b = a;
  b.move_piece(kWhiteRook, 0, 0, 0, 1);
  expect_piece(a, 0, 0, ROOK, piece_color::WHITE);
  expect_piece(b, 0, 1, ROOK, piece_color::WHITE);
}

TEST(Board, StaysConsistentInRandomGames) {
  // Many random games touch every kind of move: captures, castling, en
  // passant, promotions. After every move the board has to be consistent.
  for (uint32_t seed = 1; seed <= 20; ++seed) {
    engine e;
    e.new_game();
    random_ai ai(&e, "test", seed);
    for (int ply = 0; ply < 300 && e.get_current_gamestate().result == RUNNING;
         ++ply) {
      play(e, to_uci(ai.next_move()));
      ASSERT_TRUE(e.get_current_board().check_sanity())
          << "seed " << seed << " ply " << ply;
    }
  }
}

}  // namespace
