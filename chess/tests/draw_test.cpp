// Tests for the draw rules: stalemate, threefold repetition, 50-move rule
// and insufficient material.
#include <stdexcept>
#include <string>
#include <vector>

#include "test_helpers.hpp"

namespace {

void play_all(engine& e, const std::vector<std::string>& moves) {
  for (const std::string& m : moves) play(e, m);
}

game_result result_of(engine& e) { return e.get_current_gamestate().result; }
draw_reason reason_of(engine& e) { return e.get_current_gamestate().draw; }

// ------------------------------------------------------------- stalemate

TEST(Draw, StalemateHasItsReason) {
  engine e;
  e.new_game("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");  // black king h8, no move
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), STALEMATE);
}

TEST(Draw, RunningGameHasNoReason) {
  engine e;
  e.new_game();
  EXPECT_EQ(result_of(e), RUNNING);
  EXPECT_EQ(reason_of(e), NO_DRAW);
}

// ------------------------------------------------------ threefold repetition

TEST(Draw, ThreefoldRepetition) {
  // Both knights out and back twice: the start position for the third time.
  engine e;
  e.new_game();
  play_all(e, {"g1f3", "g8f6", "f3g1", "f6g8",  // start position: 2nd time
               "g1f3", "g8f6", "f3g1"});        // not yet the 3rd
  EXPECT_EQ(result_of(e), RUNNING);
  play(e, "f6g8");  // 3rd time
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), THREEFOLD_REPETITION);
}

TEST(Draw, RepetitionNeedsTheSameCastlingRights) {
  // Both kings walk out and back. The board of the start (A, all castling
  // rights) comes back after 4 and 8 half moves, but without castling
  // rights: a different position B. So after 8 half moves the board is there
  // for the third time, but A only once and B twice: no draw yet.
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");  // A
  play_all(e, {"e1e2", "e8e7", "e2e1", "e7e8",         // B: 1st time
               "e1e2", "e8e7", "e2e1", "e7e8"});       // B: 2nd time
  EXPECT_EQ(result_of(e), RUNNING);
  // The kings on e2 and e7 (White to move) came after 2 and 6 half moves:
  // the third time is after 10.
  play_all(e, {"e1e2", "e8e7"});
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), THREEFOLD_REPETITION);
}

TEST(Draw, RepetitionNeedsTheSameSideToMove) {
  // The white king walks a triangle h1-g1-g2-h1 (3 moves), the black king
  // goes h8-g8-h8 (2 moves). So the start board comes back after 5, 12 and
  // 17 half moves, with Black, White and Black to move: four times in all,
  // but only twice with the same side to move.
  engine e;
  e.new_game("7k/8/8/8/8/8/8/R6K w - - 0 1");
  play_all(e, {"h1g1", "h8g8", "g1g2", "g8h8", "g2h1",          // 5: Black
               "h8g8", "h1g1", "g8h8", "g1g2", "h8g8", "g2h1",  //
               "g8h8",                                          // 12: White
               "h1g1", "h8g8", "g1g2", "g8h8", "g2h1"});        // 17: Black
  EXPECT_EQ(result_of(e), RUNNING);
}

// ---------------------------------------------------------- 50-move rule

TEST(Draw, FiftyMoveRule) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K3 w - - 99 60");
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 99);
  play(e, "a1a2");  // the 100th half move without capture or pawn move
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), FIFTY_MOVE_RULE);
}

TEST(Draw, FiftyMoveClockCountsAndResets) {
  engine e;
  e.new_game("4k3/p7/8/8/8/8/P7/R3K3 w - - 10 30");
  play(e, "a1b1");  // rook move: counts
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 11);
  play(e, "a7a6");  // pawn move: starts again
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 0);
  play(e, "b1b8");
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 1);
  play(e, "e8d7");
  play(e, "b8b7");
  play(e, "d7c6");
  play(e, "b7a7");  // no capture yet
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 5);
  play(e, "c6b5");
  play(e, "a7a6");  // rook takes the pawn: starts again
  EXPECT_EQ(e.get_current_gamestate().halfmove_clock, 0);
}

TEST(Draw, MateOnTheHundredthHalfMoveWins) {
  engine e;
  e.new_game("7k/8/6K1/8/8/8/8/5Q2 w - - 99 60");
  play(e, "f1f8");
  EXPECT_EQ(result_of(e), WHITE_WIN);
  EXPECT_EQ(reason_of(e), NO_DRAW);
}

TEST(Draw, FiftyMoveRuleFromFen) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K3 w - - 100 60");
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), FIFTY_MOVE_RULE);
}

// --------------------------------------------------- insufficient material

TEST(Draw, KingTakesTheLastPiece) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/3q4/4K3 w - - 0 1");
  play(e, "e1d2");  // king against king
  EXPECT_EQ(result_of(e), DRAW);
  EXPECT_EQ(reason_of(e), INSUFFICIENT_MATERIAL);
}

struct MaterialCase {
  const char* name;
  const char* fen;
  bool draw;
};

class InsufficientMaterial : public testing::TestWithParam<MaterialCase> {};

TEST_P(InsufficientMaterial, Detected) {
  engine e;
  e.new_game(GetParam().fen);
  if (GetParam().draw) {
    EXPECT_EQ(result_of(e), DRAW);
    EXPECT_EQ(reason_of(e), INSUFFICIENT_MATERIAL);
  } else {
    EXPECT_EQ(result_of(e), RUNNING);
  }
}

INSTANTIATE_TEST_SUITE_P(
    Positions, InsufficientMaterial,
    testing::Values(
        MaterialCase{"KingAgainstKing", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", true},
        MaterialCase{"KingAndBishop", "4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", true},
        MaterialCase{"KingAndKnight", "4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", true},
        // c1 and f8 are both dark squares.
        MaterialCase{"BishopsOnTheSameColor",
                     "4kb2/8/8/8/8/8/8/2B1K3 w - - 0 1", true},
        // c1 is dark, c8 is light: a mate is possible (with help).
        MaterialCase{"BishopsOnDifferentColors",
                     "2b1k3/8/8/8/8/8/8/2B1K3 w - - 0 1", false},
        // Two knights cannot force mate, but a mate is still possible.
        MaterialCase{"TwoKnights", "4k3/8/8/8/8/8/8/1N2K1N1 w - - 0 1", false},
        MaterialCase{"KnightAgainstKnight", "1n2k3/8/8/8/8/8/8/1N2K3 w - - 0 1",
                     false},
        MaterialCase{"KingAndPawn", "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", false},
        MaterialCase{"KingAndRook", "4k3/8/8/8/8/8/8/R3K3 w - - 0 1", false}),
    [](const testing::TestParamInfo<MaterialCase>& info) {
      return std::string(info.param.name);
    });

// ------------------------------------------------------- after the draw

TEST(Draw, NoMovesAfterADraw) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K3 w - - 99 60");
  play(e, "a1a2");
  game_history gh = e.get_current_gamestate();
  EXPECT_TRUE(e.get_all_legal_moves(&gh).empty());
  move m{piece(KING, piece_color::BLACK), 4, 7, 4, 6, MOVE};  // Ke8-e7
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
}

TEST(Draw, UndoTakesTheDrawBack) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K3 w - - 99 60");
  play(e, "a1a2");
  e.undo_move();
  EXPECT_EQ(result_of(e), RUNNING);
  EXPECT_EQ(reason_of(e), NO_DRAW);
  EXPECT_TRUE(has_move(e, "a1a2"));
}

}  // namespace
