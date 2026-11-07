// Tests for the evaluation of the search (engine/scoring.hpp).
#include "../engine/scoring.hpp"

#include <string>
#include <vector>

#include "test_helpers.hpp"

namespace {

const piece_color W = piece_color::WHITE;
const piece_color B = piece_color::BLACK;

board from_fen(const std::string& fen) {
  engine e;
  e.new_game(fen);
  return e.get_current_board();
}

TEST(Scoring, StartPositionIsEven) {
  board b =
      from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  EXPECT_EQ(scoring::score(b, W), 0);
  EXPECT_EQ(scoring::score(b, B), 0);
}

TEST(Scoring, WhiteIsMinusBlack) {
  const std::vector<std::string> positions = {
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
  };
  for (const std::string& fen : positions) {
    board b = from_fen(fen);
    EXPECT_EQ(scoring::score(b, W), -scoring::score(b, B)) << fen;
  }
}

TEST(Scoring, MaterialDecides) {
  // An extra queen is worth far more than any square bonus.
  board b = from_fen("4k3/8/8/8/8/8/8/3QK3 w - - 0 1");
  EXPECT_GT(scoring::score(b, W), 800);
  EXPECT_LT(scoring::score(b, B), -800);
}

TEST(Scoring, SquaresMatter) {
  const piece knight(KNIGHT, W), pawn(PAWN, W);
  EXPECT_GT(scoring::square_bonus(knight, 3, 3),   // d4: center
            scoring::square_bonus(knight, 0, 0));  // a1: corner
  EXPECT_GT(scoring::square_bonus(pawn, 4, 6),     // e7: about to promote
            scoring::square_bonus(pawn, 4, 1));    // e2: at home
  const piece king(KING, W);
  EXPECT_GT(scoring::square_bonus(king, 6, 0),   // g1: castled
            scoring::square_bonus(king, 4, 3));  // e4: in the middle
}

TEST(Scoring, BlackUsesTheMirroredSquare) {
  // A black piece on (x, y) gets what a white piece gets on (x, 7 - y).
  for (piece_type t : {KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN}) {
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        EXPECT_EQ(scoring::square_bonus(piece(t, B), x, y),
                  scoring::square_bonus(piece(t, W), x, 7 - y))
            << "type " << int(t) << " square " << x << "," << y;
      }
    }
  }
}

}  // namespace
