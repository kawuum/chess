// Check detection: every kind of attacker, blocked lines, pins, and the tricky
// cases. Perft covers all of this in bulk; these tests show directly which
// kind of attack is broken if one fails.
#include "test_helpers.hpp"

namespace {

bool in_check(const char* fen) {
  engine e;
  e.new_game(fen);
  return e.get_current_gamestate().is_check;
}

// White king on e4 in every test, black to give check.

TEST(Check, NoCheck) {
  EXPECT_FALSE(in_check("4k3/8/8/8/4K3/8/8/8 w - - 0 1"));
}

TEST(Check, ByKnight) {
  EXPECT_TRUE(in_check("4k3/8/3n4/8/4K3/8/8/8 w - - 0 1"));   // Nd6
  EXPECT_TRUE(in_check("4k3/8/8/8/4K3/8/5n2/8 w - - 0 1"));   // Nf2
  EXPECT_FALSE(in_check("4k3/8/4n3/8/4K3/8/8/8 w - - 0 1"));  // Ne6
}

TEST(Check, ByPawnInItsDirectionOnly) {
  // A black pawn captures downwards: from d5 it attacks e4, from d3 it does
  // not.
  EXPECT_TRUE(in_check("4k3/8/8/3p4/4K3/8/8/8 w - - 0 1"));
  EXPECT_FALSE(in_check("4k3/8/8/8/4K3/3p4/8/8 w - - 0 1"));
  // A pawn straight in front does not attack.
  EXPECT_FALSE(in_check("4k3/8/8/4p3/4K3/8/8/8 w - - 0 1"));
  // White king attacked by... a white pawn never: same color.
  EXPECT_FALSE(in_check("4k3/8/8/3P4/4K3/8/8/8 w - - 0 1"));
  // Black king on e5, white pawn on d4 attacks it (upwards).
  EXPECT_TRUE(in_check("8/8/8/4k3/3P4/8/8/4K3 b - - 0 1"));
}

TEST(Check, ByBishopAndQueenOnDiagonals) {
  EXPECT_TRUE(in_check("4k3/8/8/8/4K3/8/8/1b6 w - - 0 1"));   // Bb1
  EXPECT_TRUE(in_check("4k3/7q/8/8/4K3/8/8/8 w - - 0 1"));    // Qh7
  EXPECT_FALSE(in_check("4k3/8/8/8/4K3/8/8/1r6 w - - 0 1"));  // rook on diag
}

TEST(Check, ByRookAndQueenOnLines) {
  EXPECT_TRUE(in_check("4k3/8/8/8/r3K3/8/8/8 w - - 0 1"));   // Ra4
  EXPECT_TRUE(in_check("4k3/8/8/8/4K3/8/8/4q3 w - - 0 1"));  // Qe1
  EXPECT_FALSE(in_check("4k3/8/8/8/b3K3/8/8/8 w - - 0 1"));  // bishop on line
}

TEST(Check, BlockedLineIsNoCheck) {
  EXPECT_FALSE(in_check("4k3/8/8/8/r1N1K3/8/8/8 w - - 0 1"));   // Nc4 blocks
  EXPECT_FALSE(in_check("4k3/8/8/8/4K3/3P4/8/1b6 w - - 0 1"));  // Pd3 blocks
}

TEST(Check, KingsMayNotTouch) {
  // The rook on a1 is only there so the position is no draw: king against
  // king alone is insufficient material, and then there are no moves.
  engine e;
  e.new_game("8/8/8/4k3/8/4K3/8/R7 w - - 0 1");
  EXPECT_FALSE(has_move(e, "e3e4"));
  EXPECT_FALSE(has_move(e, "e3d4"));
  EXPECT_TRUE(has_move(e, "e3e2"));
}

TEST(Check, PinnedPieceCannotMove) {
  // The white bishop on e2 is pinned by the black rook on e8.
  engine e;
  e.new_game("4r1k1/8/8/8/8/8/4B3/4K3 w - - 0 1");
  EXPECT_TRUE(e.get_legal_moves(4, 1).empty());
}

TEST(Check, KingCannotTakeProtectedPiece) {
  // Black queen on e2 next to the white king, protected by the rook on e8.
  engine e;
  e.new_game("4r1k1/8/8/8/8/8/4q3/4K3 w - - 0 1");
  EXPECT_FALSE(has_move(e, "e1e2"));
}

TEST(Check, EnPassantThatExposesTheKing) {
  // After e5xd6 e.p. both pawns leave rank 5, and the rook on h5 would hit
  // the king on a5. So the en passant capture is illegal.
  engine e;
  e.new_game("8/8/8/K2pP2r/8/8/8/7k w - d6 0 1");
  EXPECT_FALSE(has_move(e, "e5d6"));
  EXPECT_TRUE(has_move(e, "e5e6"));
}

TEST(Check, DoubleCheckOnlyKingMoves) {
  // Rook on e8 and knight on f3 both give check: blocking or capturing one of
  // them is not enough, only the king may move.
  engine e;
  e.new_game("4r1k1/8/8/8/8/5n2/3B4/4K3 w - - 0 1");
  EXPECT_TRUE(e.get_current_gamestate().is_check);
  for (const std::string& m : legal_uci_moves(e)) {
    EXPECT_EQ(m.substr(0, 2), "e1") << m;
  }
}

}  // namespace
