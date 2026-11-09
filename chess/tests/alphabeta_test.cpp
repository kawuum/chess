// Tests for alphabeta_ai: the same results as minimax_ai with fewer
// positions, and the positions where looking ahead decides.
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <random>
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

// The same comparison in positions from random games. The same random
// moves are played on three engines, so all three know the same game (a
// repetition in the search counts the same everywhere). Every 4th position
// is searched 3 half moves deep, the others 2.
void compare_in_random_positions(int positions) {
  int compared = 0;
  for (uint32_t game = 1; compared < positions; ++game) {
    engine e1, e2, e3;
    for (engine* e : {&e1, &e2, &e3}) e->new_game();
    std::mt19937 rng(game);
    for (int ply = 0; ply < 80 && compared < positions; ++ply) {
      const std::vector<move> moves = e1.get_all_legal_moves();
      if (moves.empty()) break;  // the game is over
      const move m = moves[rng() % moves.size()];
      for (engine* e : {&e1, &e2, &e3}) e->perform_move(m);
      if (ply % 8 != 7 || e1.current_state().result != RUNNING) continue;

      const int depth = compared % 4 == 0 ? 3 : 2;
      const uint32_t seed = static_cast<uint32_t>(compared);
      minimax_ai minimax(&e1, "minimax", seed, depth);
      alphabeta_ai sorted(&e2, "sorted", seed, depth);
      alphabeta_ai unsorted(&e3, "unsorted", seed, depth, false);
      const move m1 = minimax.next_move();
      sorted.next_move();
      const move m3 = unsorted.next_move();
      const std::string where = "game " + std::to_string(game) + " ply " +
                                std::to_string(ply) + " depth " +
                                std::to_string(depth);
      for (const alphabeta_ai* ab : {&sorted, &unsorted}) {
        ASSERT_EQ(ab->best_score(), minimax.best_score()) << where;
        ASSERT_EQ(sorted_uci(ab->best_moves()),
                  sorted_uci(minimax.best_moves()))
            << where;
        EXPECT_LE(ab->positions_searched(), minimax.positions_searched())
            << where;
      }
      ASSERT_EQ(to_uci(m3), to_uci(m1)) << where;
      ++compared;
    }
  }
}

// 40 positions: about 3 s in the debug build.
TEST(AlphaBeta, SameResultAsMinimaxInRandomPositions) {
  compare_in_random_positions(40);
}

// 400 positions: about 5 s in the release build, only with
// CHESS_SLOW_TESTS=1.
TEST(DeepAlphaBeta, SameResultAsMinimaxInManyRandomPositions) {
  if (std::getenv("CHESS_SLOW_TESTS") == nullptr) {
    GTEST_SKIP() << "set CHESS_SLOW_TESTS=1 to compare in 400 positions";
  }
  compare_in_random_positions(400);
}

// Scores have the right sign for both colors and count the half moves to
// a mate, for both searches.
TEST(AlphaBeta, ScoresHaveTheRightSignAndMateDistance) {
  struct Case {
    const char* fen;
    const char* first;  // a move played before the search, or ""
    int min_score, max_score;
  };
  constexpr int kMate = alphabeta_ai::kMateScore;
  const Case cases[] = {
      // A queen up, White or Black to move: clearly positive.
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", "", 700, 1100},
      {"3qk3/8/8/8/8/8/8/4K3 b - - 0 1", "", 700, 1100},
      // A queen down: clearly negative.
      {"3qk3/8/8/8/8/8/8/4K3 w - - 0 1", "", -1100, -700},
      {"4k3/8/8/8/8/8/8/3QK3 b - - 0 1", "", -1100, -700},
      // Mate in two for White: mate after 3 half moves.
      {"7k/8/8/8/8/8/1R6/R3K3 w - - 0 1", "", kMate - 3, kMate - 3},
      // After Rb7 Black is mated in 2 half moves whatever it plays.
      {"7k/8/8/8/8/8/1R6/R3K3 w - - 0 1", "b2b7", -(kMate - 2), -(kMate - 2)},
      // The same with colors swapped (board mirrored).
      {"r3k3/1r6/8/8/8/8/8/7K b - - 0 1", "", kMate - 3, kMate - 3},
      {"r3k3/1r6/8/8/8/8/8/7K b - - 0 1", "b7b2", -(kMate - 2), -(kMate - 2)},
  };
  for (const Case& c : cases) {
    engine e1, e2;
    e1.new_game(c.fen);
    e2.new_game(c.fen);
    if (c.first[0] != '\0') {
      play(e1, c.first);
      play(e2, c.first);
    }
    minimax_ai minimax(&e1, "minimax", 1, 3);
    alphabeta_ai ab(&e2, "alphabeta", 1, 3);
    minimax.next_move();
    ab.next_move();
    for (int score : {minimax.best_score(), ab.best_score()}) {
      EXPECT_GE(score, c.min_score) << c.fen << " " << c.first;
      EXPECT_LE(score, c.max_score) << c.fen << " " << c.first;
    }
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

// A plain number as third argument is the seed, not the depth (see
// Minimax.ThirdArgumentIsTheSeed).
TEST(AlphaBeta, ThirdArgumentIsTheSeed) {
  engine e;
  e.new_game();
  alphabeta_ai ai(&e, "alphabeta", 1);
  EXPECT_EQ(ai.depth(), alphabeta_ai::kDefaultDepth);
  alphabeta_ai deeper(&e, "alphabeta", 1, 2);
  EXPECT_EQ(deeper.depth(), 2);
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
  EXPECT_TRUE(ai.best_moves().empty());
  // "No score": a value whose negation is safe (INT_MIN would overflow).
  EXPECT_EQ(ai.best_score(), -alphabeta_ai::kInfinity);
}

TEST(AlphaBeta, DepthBelowOneSearchesOneHalfMove) {
  // Depth 0 searched forever before: the search stops at depth == 0, and
  // the first step down went to -1.
  engine e;
  e.new_game();
  alphabeta_ai ai(&e, "alphabeta", 1, 0);
  ASSERT_EQ(ai.depth(), 1);
  EXPECT_NE(ai.next_move().type, NO_MOVE);
  alphabeta_ai negative(&e, "alphabeta", 1, -5);
  EXPECT_EQ(negative.depth(), 1);
}

// Strength in the normal test run: alpha-beta 3 half moves deep against the
// naive AI (which looks 1 half move ahead), 6 games with colors swapping.
// Catches a search that got much weaker (e.g. a sign error) without the
// slow tests. Won 6 of 6 when it was written.
TEST(AlphaBeta, QuicklyBeatsTheNaiveAi) {
  const ai_info* naive = nullptr;
  for (const ai_info& info : available_ais()) {
    if (info.name == "Naive AI") naive = &info;
  }
  ASSERT_NE(naive, nullptr);
  const ai_info shallow{"Alpha-Beta 3", [](engine* e, uint32_t seed) {
                          return std::make_unique<alphabeta_ai>(
                              e, "Alpha-Beta 3", seed, 3);
                        }};
  match_settings settings;
  settings.games = 6;
  settings.max_plies = 300;
  settings.first_seed = 5;
  settings.second_seed = 6;
  match_stats s = play_match(shallow, *naive, settings);
  EXPECT_GE(s.first_wins, 5) << describe(s, "Alpha-Beta 3", "Naive AI");
  EXPECT_EQ(s.second_wins, 0) << describe(s, "Alpha-Beta 3", "Naive AI");
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
