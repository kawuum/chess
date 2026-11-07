// Tests for minimax_ai: positions where looking ahead decides, the rules
// every AI must follow, and (slow) its strength against the other AIs.
#include <cstdlib>
#include <string>
#include <vector>

#include "../ai/ai_registry.hpp"
#include "../ai/match.hpp"
#include "../ai/minimax/minimax_ai.hpp"
#include "test_helpers.hpp"

namespace {

std::string best_move(const std::string& fen, uint32_t seed = 1) {
  engine e;
  e.new_game(fen);
  minimax_ai ai(&e, "minimax", seed);
  return to_uci(ai.next_move());
}

TEST(Minimax, TakesMateInOne) {
  // Several queen moves mate; Qf7 would be stalemate.
  for (uint32_t seed = 1; seed <= 3; ++seed) {
    engine e;
    e.new_game("7k/8/6K1/8/8/8/8/5Q2 w - - 0 1");
    minimax_ai ai(&e, "minimax", seed);
    e.perform_move(ai.next_move());
    EXPECT_EQ(e.get_current_gamestate().result, WHITE_WIN) << "seed " << seed;
  }
}

TEST(Minimax, FindsMateInTwo) {
  // Black king h8, white rooks a1 and b2: a check now does not mate (the king
  // escapes to rank 7). First a quiet rook move closes rank 7 (Ra7 or Rb7),
  // then the other rook mates on rank 8 - whatever Black does in between.
  // Only a search of 3 half moves can see that.
  engine e;
  e.new_game("7k/8/8/8/8/8/1R6/R3K3 w - - 0 1");
  minimax_ai ai(&e, "minimax", 1);
  e.perform_move(ai.next_move());
  ASSERT_EQ(e.get_current_gamestate().result, RUNNING);
  game_history gh = e.get_current_gamestate();
  const std::vector<move> replies = e.get_all_legal_moves(&gh);
  ASSERT_FALSE(replies.empty());
  for (const move& reply : replies) {
    e.perform_move(reply);
    e.perform_move(ai.next_move());
    EXPECT_EQ(e.get_current_gamestate().result, WHITE_WIN)
        << "after the black reply " << to_uci(reply);
    e.undo_move();
    e.undo_move();
  }
}

TEST(Minimax, DefendsAgainstMateInOne) {
  // Black threatens the back rank mate Re1# (the white king on g1 is boxed
  // in by its own pawns). White must do something about it, e.g. make room
  // for the king (h3) or block on e3. Afterwards no black move may mate.
  // A search with a wrong sign (opponent not minimizing) fails here.
  for (uint32_t seed = 1; seed <= 3; ++seed) {
    engine e;
    e.new_game("4r1k1/5ppp/8/8/8/R7/5PPP/6K1 w - - 0 1");
    minimax_ai ai(&e, "minimax", seed);
    move m = ai.next_move();
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

TEST(Minimax, TakesAnUndefendedQueen) {
  EXPECT_EQ(best_move("4k3/8/8/3q4/4P3/8/8/4K3 w - - 0 1"), "e4d5");
}

TEST(Minimax, SeesTheRecapture) {
  // Qxc5 would win a pawn, but the pawn d6 takes back the queen. The naive
  // AI (one half move) cannot see that; minimax does.
  for (uint32_t seed = 1; seed <= 3; ++seed) {
    EXPECT_NE(best_move("4k3/8/3p4/2p5/3Q4/8/8/4K3 w - - 0 1", seed), "d4c5")
        << "seed " << seed;
  }
}

TEST(Minimax, LeavesThePositionUnchanged) {
  engine e;
  e.new_game(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq "
      "- 0 1");
  board before = e.get_current_board();
  minimax_ai ai(&e, "minimax", 1, 2);  // depth 2 keeps the debug build fast
  ai.next_move();
  EXPECT_TRUE(e.get_current_board() == before);
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 0);
  EXPECT_GT(ai.positions_searched(), 2000);  // 48 moves, about 2000 answers
}

TEST(Minimax, NoMoveWhenGameIsOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  minimax_ai ai(&e, "minimax", 1);
  EXPECT_EQ(ai.next_move().type, NO_MOVE);
}

// Strength: about 20 s in the release build, so only with CHESS_SLOW_TESTS=1.
TEST(DeepMinimax, BeatsTheNaiveAi) {
  if (std::getenv("CHESS_SLOW_TESTS") == nullptr) {
    GTEST_SKIP() << "set CHESS_SLOW_TESTS=1 to run the long match";
  }
  const ai_info* minimax = nullptr;
  const ai_info* naive = nullptr;
  for (const ai_info& info : available_ais()) {
    if (info.name == "Minimax AI") minimax = &info;
    if (info.name == "Naive AI") naive = &info;
  }
  ASSERT_NE(minimax, nullptr);
  ASSERT_NE(naive, nullptr);
  match_settings settings;
  settings.games = 10;
  settings.swap_colors = true;  // half the games with each color
  settings.first_seed = 11;
  settings.second_seed = 22;
  match_stats s = play_match(*minimax, *naive, settings);
  EXPECT_GE(s.first_wins, 9);  // 40 of 40 when it was written
  EXPECT_EQ(s.second_wins, 0);
}

}  // namespace
