// Tests for the evaluation building blocks (engine/evaluation.hpp).
#include "../engine/evaluation.hpp"

#include <string>
#include <vector>

#include "../engine/move_generation.hpp"
#include "test_helpers.hpp"

namespace {

using evaluation::attacked_squares;
using evaluation::attackers;
using evaluation::defended_pieces;
using evaluation::material;
using evaluation::threatened_pieces;
using evaluation::threatened_value;

const piece_color W = piece_color::WHITE;
const piece_color B = piece_color::BLACK;

board from_fen(const std::string& fen) {
  engine e;
  e.new_game(fen);
  return e.get_current_board();
}

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// "d4" -> true if that square is set in m.
bool has(const bit_matrix& m, const char* square) {
  return m.get_bit_at(square[0] - 'a', square[1] - '1');
}

int attackers_of(const board& b, const char* square, piece_color c) {
  return attackers(b, square[0] - 'a', square[1] - '1', c);
}

TEST(Evaluation, PieceValues) {
  EXPECT_EQ(evaluation::piece_value(QUEEN), 9);
  EXPECT_EQ(evaluation::piece_value(ROOK), 5);
  EXPECT_EQ(evaluation::piece_value(BISHOP), 3);
  EXPECT_EQ(evaluation::piece_value(KNIGHT), 3);
  EXPECT_EQ(evaluation::piece_value(PAWN), 1);
  EXPECT_EQ(evaluation::piece_value(KING), 0);
}

TEST(Evaluation, MaterialStartPosition) {
  board b = from_fen(kStart);
  EXPECT_EQ(material(b, W), 39);  // 8 + 2*3 + 2*3 + 2*5 + 9
  EXPECT_EQ(material(b, B), 39);
}

TEST(Evaluation, MaterialCountsOnlyOneColor) {
  board b = from_fen("4k3/8/8/8/8/8/8/R3K2q w - - 0 1");
  EXPECT_EQ(material(b, W), 5);
  EXPECT_EQ(material(b, B), 9);
}

TEST(Evaluation, CoveredSquaresStartPosition) {
  // White covers rank 3 (pawns and knights), all of rank 2 and b1-g1: 22.
  board b = from_fen(kStart);
  bit_matrix white = attacked_squares(b, W);
  EXPECT_EQ(white.get_num_bits_set(), 22);
  EXPECT_EQ(attacked_squares(b, B).get_num_bits_set(), 22);
  EXPECT_TRUE(has(white, "a3"));
  EXPECT_TRUE(has(white, "h3"));
  EXPECT_FALSE(has(white, "a4"));
  EXPECT_FALSE(has(white, "a1"));  // nothing covers the rook in the corner
}

TEST(Evaluation, AttackersStartPosition) {
  board b = from_fen(kStart);
  EXPECT_EQ(attackers_of(b, "d2", W), 4);  // Nb1, Bc1, Qd1, Ke1
  EXPECT_EQ(attackers_of(b, "e2", W), 4);  // Qd1, Ke1, Bf1, Ng1
  EXPECT_EQ(attackers_of(b, "f3", W), 3);  // pawns e2 and g2, Ng1
  EXPECT_EQ(attackers_of(b, "e4", W), 0);
}

TEST(Evaluation, RayStopsAtTheFirstPiece) {
  // White rook d4, own pawn d6: the rook covers d6 (defends it), not d7.
  board b = from_fen("4k3/8/3P4/8/3R4/8/8/4K3 w - - 0 1");
  bit_matrix white = attacked_squares(b, W);
  EXPECT_TRUE(has(white, "d5"));
  EXPECT_TRUE(has(white, "d6"));
  EXPECT_FALSE(has(white, "d7"));
  EXPECT_FALSE(has(white, "d8"));
  EXPECT_TRUE(has(white, "a4"));
  EXPECT_TRUE(has(white, "h4"));
}

TEST(Evaluation, PawnsCoverDiagonallyOnly) {
  board b = from_fen("4k3/8/8/4p3/4P3/8/8/4K3 w - - 0 1");
  bit_matrix white = attacked_squares(b, W);
  EXPECT_TRUE(has(white, "d5"));
  EXPECT_TRUE(has(white, "f5"));
  EXPECT_FALSE(has(white, "e5"));  // straight ahead: no capture
  bit_matrix black = attacked_squares(b, B);
  EXPECT_TRUE(has(black, "d4"));
  EXPECT_TRUE(has(black, "f4"));
  EXPECT_FALSE(has(black, "e4"));
  EXPECT_EQ(threatened_pieces(b, W).get_num_bits_set(), 0);
}

TEST(Evaluation, OnlyTheFirstPieceOnALineCounts) {
  // Queen d1 behind the rook d2: only the rook covers d5.
  board b = from_fen("4k3/8/8/8/8/8/3R4/3Q1K2 w - - 0 1");
  EXPECT_EQ(attackers_of(b, "d5", W), 1);
  EXPECT_EQ(attackers_of(b, "d2", W), 1);  // the queen defends the rook
}

TEST(Evaluation, KnightForkThreatensRookAndKing) {
  // Black to move: with White to move, Black would have left its king in
  // check, and the engine rejects that FEN.
  board b = from_fen("r3k3/2N5/8/8/8/8/8/4K3 b - - 0 1");
  bit_matrix threatened = threatened_pieces(b, W);
  EXPECT_EQ(threatened.get_num_bits_set(), 2);
  EXPECT_TRUE(has(threatened, "a8"));
  EXPECT_TRUE(has(threatened, "e8"));    // the king in check counts ...
  EXPECT_EQ(threatened_value(b, W), 5);  // ... but with the value 0
  EXPECT_EQ(threatened_value(b, B), 0);  // nothing reaches the knight
}

TEST(Evaluation, DefendedPieces) {
  // Pawn c3 defends pawn d4, not the other way round.
  board b = from_fen("4k3/8/8/8/3P4/2P5/8/4K3 w - - 0 1");
  bit_matrix defended = defended_pieces(b, W);
  EXPECT_EQ(defended.get_num_bits_set(), 1);
  EXPECT_TRUE(has(defended, "d4"));
}

TEST(Evaluation, DefendedPiecesStartPosition) {
  // All of rank 2, and b1-g1; the rooks in the corners are not covered.
  board b = from_fen(kStart);
  EXPECT_EQ(defended_pieces(b, W).get_num_bits_set(), 14);
  EXPECT_EQ(defended_pieces(b, B).get_num_bits_set(), 14);
}

// The three ways to ask "is this square covered?" must agree on every square:
// attacked_squares(), attackers() > 0 and move_generation::is_square_attacked
// (which perft has checked millions of times).
TEST(Evaluation, AgreesWithIsSquareAttacked) {
  const std::vector<std::string> positions = {
      kStart,
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
      "10",
      "4k3/8/8/8/8/8/3R4/3Q1K2 w - - 0 1",
  };
  for (const std::string& fen : positions) {
    board b = from_fen(fen);
    for (piece_color c : {W, B}) {
      bit_matrix covered = attacked_squares(b, c);
      for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
          bool expected = move_generation::is_square_attacked(b, x, y, c);
          EXPECT_EQ(covered.get_bit_at(x, y), expected)
              << fen << " square " << char('a' + x) << y + 1;
          EXPECT_EQ(attackers(b, x, y, c) > 0, expected)
              << fen << " square " << char('a' + x) << y + 1;
        }
      }
    }
  }
}

}  // namespace
