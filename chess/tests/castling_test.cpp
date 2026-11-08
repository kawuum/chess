// Castling rules: when castling is allowed, and when a castling right is lost.
#include "test_helpers.hpp"

namespace {

castling_rights rights(engine& e, piece_color c) {
  return e.get_current_gamestate().castlingrights[(int)c];
}

// --- Move generation ---------------------------------------------------------

TEST(Castling, BothSidesAllowedOnEmptyBackRank) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  EXPECT_TRUE(has_move(e, "e1g1"));
  EXPECT_TRUE(has_move(e, "e1c1"));
}

TEST(Castling, LongCastlingBlockedByPieceOnB1) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/RN2K3 w Q - 0 1");
  EXPECT_FALSE(has_move(e, "e1c1"));
}

TEST(Castling, LongCastlingAllowedWhenB1IsAttacked) {
  // The king never crosses b1, so an attack on b1 does not matter.
  engine e;
  e.new_game("1r2k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
  EXPECT_TRUE(has_move(e, "e1c1"));
}

TEST(Castling, NotAllowedThroughAttackedSquare) {
  // The black rook on f8 attacks f1, which the king has to cross.
  engine e;
  e.new_game("4kr2/8/8/8/8/8/8/4K2R w K - 0 1");
  EXPECT_FALSE(has_move(e, "e1g1"));
}

TEST(Castling, RightWithoutRookIsRejected) {
  // Inconsistent FEN: right K but no rook on h1. Before the FEN check the
  // engine loaded it (and only the move generator kept it from castling).
  engine e;
  EXPECT_EQ(e.try_new_game("4k3/8/8/8/8/8/8/4K3 w K - 0 1"),
            fen_error::CASTLING_PIECES);
}

TEST(Castling, MovesKingAndRook) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  play(e, "e1g1");
  board& b = e.get_current_board();
  EXPECT_EQ(b.get_piece(6, 0).get_piece_type(), KING);  // g1
  EXPECT_EQ(b.get_piece(5, 0).get_piece_type(), ROOK);  // f1
  EXPECT_FALSE(b.get_piece(7, 0).is_valid());           // h1 empty
}

// --- Losing castling rights --------------------------------------------------

TEST(CastlingRights, KingMoveLosesBoth) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  play(e, "e1e2");
  EXPECT_EQ(rights(e, piece_color::WHITE), NONE);
  EXPECT_EQ(rights(e, piece_color::BLACK), BOTH);
}

TEST(CastlingRights, RookMoveLosesOneSide) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  play(e, "h1h2");
  EXPECT_EQ(rights(e, piece_color::WHITE), LONG);
}

TEST(CastlingRights, CapturedRookInCornerLosesRight) {
  // Bb2xh8: black may no longer castle short, there is no rook.
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/1B6/4K3 w kq - 0 1");
  play(e, "b2h8");
  EXPECT_EQ(rights(e, piece_color::BLACK), LONG);
  EXPECT_FALSE(has_move(e, "e8g8"));
}

TEST(CastlingRights, CaptureOnOwnBackRankKeepsOpponentRights) {
  // Qd1xa1 takes a black rook on a1. The black rooks on a8 and h8 never
  // moved, so black keeps both rights.
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/r2QK3 w kq - 0 1");
  play(e, "d1a1");
  EXPECT_EQ(rights(e, piece_color::BLACK), BOTH);
}

TEST(CastlingRights, RookTakesRookInOppositeCorner) {
  // Ra1xa8: white loses the long right (rook left a1), black loses the long
  // right (rook captured on a8).
  engine e;
  e.new_game("r3k3/8/8/8/8/8/8/R3K3 w Qq - 0 1");
  play(e, "a1a8");
  EXPECT_EQ(rights(e, piece_color::WHITE), NONE);
  EXPECT_EQ(rights(e, piece_color::BLACK), NONE);
}

}  // namespace
