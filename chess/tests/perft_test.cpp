// Perft tests: count all move sequences from well-known positions and compare
// with the published numbers (https://www.chessprogramming.org/Perft_Results).
// A wrong count means the move generator creates illegal moves or misses
// legal ones.
#include <cstdlib>

#include "test_helpers.hpp"

namespace {

struct PerftCase {
  const char* name;
  const char* fen;
  std::vector<uint64_t> nodes;  // expected counts for depth 1, 2, 3, ...
};

// Lets GoogleTest print a case by its name instead of its raw bytes.
void PrintTo(const PerftCase& c, std::ostream* os) { *os << c.name; }

// Depths with up to this many nodes run in every test run (a few seconds).
// Deeper ones only run with CHESS_SLOW_TESTS=1 set in the environment.
constexpr uint64_t kFastLimit = 100000;

class Perft : public testing::TestWithParam<PerftCase> {};

void check_depths(const PerftCase& c, bool deep) {
  engine e;
  e.new_game(c.fen);
  for (size_t i = 0; i < c.nodes.size(); ++i) {
    bool fast = c.nodes[i] <= kFastLimit;
    if (fast == deep) continue;
    int depth = static_cast<int>(i) + 1;
    EXPECT_EQ(perft(e, depth), c.nodes[i]) << c.name << " depth " << depth;
  }
}

TEST_P(Perft, Fast) { check_depths(GetParam(), false); }

TEST_P(Perft, Deep) {
  if (std::getenv("CHESS_SLOW_TESTS") == nullptr) {
    GTEST_SKIP() << "set CHESS_SLOW_TESTS=1 to run deep perft";
  }
  check_depths(GetParam(), true);
}

INSTANTIATE_TEST_SUITE_P(
    Positions, Perft,
    testing::Values(
        PerftCase{"start",
                  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                  {20, 400, 8902, 197281}},
        // Endgame with en passant, checks and pins.
        PerftCase{"position3",
                  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                  {14, 191, 2812, 43238, 674624}},
        // Middlegame, both sides castled.
        PerftCase{"position6",
                  "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/"
                  "R4RK1 w - - 0 10",
                  {46, 2079, 89890, 3894594}}),
    [](const testing::TestParamInfo<PerftCase>& info) {
      return std::string(info.param.name);
    });

TEST(PerftHelpers, DivideSumsUpToPerft) {
  engine e;
  e.new_game();
  uint64_t sum = 0;
  for (const auto& entry : perft_divide(e, 2)) sum += entry.second;
  EXPECT_EQ(sum, perft(e, 2));
}

TEST(PerftHelpers, PerftLeavesThePositionUnchanged) {
  engine e;
  e.new_game();
  perft(e, 3);
  game_history gs = e.get_current_gamestate();
  EXPECT_EQ(gs.num_halfmoves, 0);
  EXPECT_EQ(legal_uci_moves(e).size(), 20u);
}

TEST(PerftHelpers, UciNotation) {
  move m{piece(PAWN, piece_color::WHITE), 4, 1, 4, 3, MOVE};
  EXPECT_EQ(to_uci(m), "e2e4");
  move promo{piece(PAWN, piece_color::WHITE), 0, 6, 0, 7, PROMOTION};
  EXPECT_EQ(to_uci(promo, KNIGHT), "a7a8n");
}

}  // namespace
