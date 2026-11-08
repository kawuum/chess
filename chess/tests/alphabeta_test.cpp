// Tests for alphabeta_ai: the same results as minimax_ai with fewer
// positions, and the positions where looking ahead decides.
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "../ai/ai_registry.hpp"
#include "../ai/alphabeta/alphabeta_ai.hpp"
#include "../ai/match.hpp"
#include "../ai/minimax/minimax_ai.hpp"
#include "test_helpers.hpp"

namespace {

std::vector<std::string> sorted_uci(const std::vector<move>& moves) {
  std::vector<std::string> result;
  for (const move& m : moves) result.push_back(to_uci(m));
  std::sort(result.begin(), result.end());
  return result;
}

struct Comparison {
  const char* fen;
  int depth;
};

const Comparison kCases[] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3},
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2},
    {"4r1k1/5ppp/8/8/8/R7/5PPP/6K1 w - - 0 1", 3},  // mate threat
    {"7k/8/8/8/8/8/1R6/R3K3 w - - 0 1", 3},         // mate in two
    {"4k3/8/3p4/2p5/3Q4/8/8/4K3 w - - 0 1", 3},     // recapture
};

// Alpha-beta must find exactly what minimax finds: the same score and the
// same set of equally best moves, while looking at fewer positions. Without
// move ordering it even searches the moves in the same order, so the same
// seed picks the same move.
TEST(AlphaBeta, SameResultAsMinimaxWithFewerPositions) {
  for (const Comparison& c : kCases) {
    engine e1, e2, e3;
    e1.new_game(c.fen);
    e2.new_game(c.fen);
    e3.new_game(c.fen);
    minimax_ai minimax(&e1, "minimax", 7, c.depth);
    alphabeta_ai sorted(&e2, "sorted", 7, c.depth);
    alphabeta_ai unsorted(&e3, "unsorted", 7, c.depth, /*sort_moves=*/false);
    const move m1 = minimax.next_move();
    sorted.next_move();
    const move m3 = unsorted.next_move();
    for (const alphabeta_ai* ab : {&sorted, &unsorted}) {
      EXPECT_EQ(ab->best_score(), minimax.best_score()) << c.fen;
      EXPECT_EQ(sorted_uci(ab->best_moves()), sorted_uci(minimax.best_moves()))
          << c.fen;
      EXPECT_LT(ab->positions_searched(), minimax.positions_searched())
          << c.fen;
    }
    EXPECT_EQ(to_uci(m3), to_uci(m1)) << c.fen;
  }
}

TEST(AlphaBeta, MoveOrderingSavesPositions) {
  // Captures first (most valuable victim first) make cutoffs come earlier.
  long long sorted_total = 0, unsorted_total = 0;
  for (const Comparison& c : kCases) {
    engine e1, e2;
    e1.new_game(c.fen);
    e2.new_game(c.fen);
    alphabeta_ai sorted(&e1, "sorted", 7, c.depth);
    alphabeta_ai unsorted(&e2, "unsorted", 7, c.depth, /*sort_moves=*/false);
    sorted.next_move();
    unsorted.next_move();
    sorted_total += sorted.positions_searched();
    unsorted_total += unsorted.positions_searched();
  }
  EXPECT_LT(sorted_total * 10, unsorted_total * 8)  // at least 20% fewer
      << sorted_total << " vs " << unsorted_total;
}

TEST(AlphaBeta, TakesMateInOne) {
  engine e;
  e.new_game("7k/8/6K1/8/8/8/8/5Q2 w - - 0 1");
  alphabeta_ai ai(&e, "alphabeta", 1);
  e.perform_move(ai.next_move());
  EXPECT_EQ(e.get_current_gamestate().result, WHITE_WIN);
}

TEST(AlphaBeta, DefendsAgainstMateInOne) {
  for (uint32_t seed = 1; seed <= 3; ++seed) {
    engine e;
    e.new_game("4r1k1/5ppp/8/8/8/R7/5PPP/6K1 w - - 0 1");
    alphabeta_ai ai(&e, "alphabeta", seed);
    const move m = ai.next_move();
    e.perform_move(m);
    game_history gh = e.get_current_gamestate();
    for (const move& reply : e.get_all_legal_moves(&gh)) {
      e.perform_move(reply);
      EXPECT_NE(e.get_current_gamestate().result, BLACK_WIN)
          << "seed " << seed << ": after " << to_uci(m) << ", " << to_uci(reply)
          << " mates";
      e.undo_move();
    }
  }
}

TEST(AlphaBeta, LeavesThePositionUnchanged) {
  engine e;
  e.new_game(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  board before = e.get_current_board();
  alphabeta_ai ai(&e, "alphabeta", 1, 2);
  ai.next_move();
  EXPECT_TRUE(e.get_current_board() == before);
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 0);
}

TEST(AlphaBeta, NoMoveWhenGameIsOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  alphabeta_ai ai(&e, "alphabeta", 1);
  EXPECT_EQ(ai.next_move().type, NO_MOVE);
}

// Strength: a few seconds in the release build, only with CHESS_SLOW_TESTS=1.
// (Against the minimax AI it won 17 of 20 games with 2 draws, but those games
// take minutes because minimax itself is slow.)
TEST(DeepAlphaBeta, BeatsTheNaiveAi) {
  if (std::getenv("CHESS_SLOW_TESTS") == nullptr) {
    GTEST_SKIP() << "set CHESS_SLOW_TESTS=1 to run the long match";
  }
  const ai_info* alphabeta = nullptr;
  const ai_info* naive = nullptr;
  for (const ai_info& info : available_ais()) {
    if (info.name == "Alpha-Beta AI") alphabeta = &info;
    if (info.name == "Naive AI") naive = &info;
  }
  ASSERT_NE(alphabeta, nullptr);
  ASSERT_NE(naive, nullptr);
  match_settings settings;
  settings.games = 10;
  settings.swap_colors = true;
  settings.first_seed = 31;
  settings.second_seed = 47;
  match_stats s = play_match(*alphabeta, *naive, settings);
  EXPECT_GE(s.first_wins, 9);  // 20 of 20 when it was written
  EXPECT_EQ(s.second_wins, 0);
}

}  // namespace
