// Tests for random_ai: legal moves only, reproducible with a fixed seed.
#include "../ai/random/random_ai.hpp"

#include "test_helpers.hpp"

namespace {

// Lets the AI play both sides for up to max_plies half moves and returns the
// moves in UCI notation. Every move must be legal.
std::vector<std::string> self_play(random_ai& ai, engine& e, int max_plies) {
  std::vector<std::string> played;
  for (int ply = 0; ply < max_plies; ++ply) {
    if (e.get_current_gamestate().result != RUNNING) break;
    move m = ai.next_move();
    std::string uci = to_uci(m);
    EXPECT_TRUE(has_move(e, uci)) << "illegal move " << uci;
    play(e, uci);
    played.push_back(uci);
  }
  return played;
}

TEST(RandomAi, PlaysOnlyLegalMoves) {
  engine e;
  e.new_game();
  random_ai ai(&e);
  std::vector<std::string> played = self_play(ai, e, 200);
  EXPECT_FALSE(played.empty());
}

TEST(RandomAi, SameSeedPlaysSameGame) {
  engine e1, e2;
  e1.new_game();
  e2.new_game();
  random_ai a(&e1, "a", 42), b(&e2, "b", 42);
  EXPECT_EQ(self_play(a, e1, 60), self_play(b, e2, 60));
}

TEST(RandomAi, InstancesCreatedTogetherPlayDifferentGames) {
  // With srand(time(NULL)) both AIs got the same seed and played the same
  // game. Now each instance has its own random seed.
  engine e1, e2;
  e1.new_game();
  e2.new_game();
  random_ai a(&e1), b(&e2);
  EXPECT_NE(self_play(a, e1, 60), self_play(b, e2, 60));
}

TEST(RandomAi, NoMoveWhenGameIsOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  random_ai ai(&e);
  EXPECT_EQ(ai.next_move().type, NO_MOVE);
}

}  // namespace
