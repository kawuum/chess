// Tests that hold for every AI in the registry, also for future ones.
#include "../ai/ai_registry.hpp"

#include <string>
#include <vector>

#include "../ai/alphabeta/alphabeta_ai.hpp"
#include "../ai/minimax/minimax_ai.hpp"
#include "test_helpers.hpp"

namespace {

// The AI plays both sides for up to max_plies half moves; returns the moves.
std::vector<std::string> self_play(const ai_info& info, uint32_t seed,
                                   int max_plies) {
  engine e;
  e.new_game();
  std::unique_ptr<ai_template> ai = info.create(&e, seed);
  std::vector<std::string> played;
  for (int ply = 0; ply < max_plies; ++ply) {
    if (e.get_current_gamestate().result != RUNNING) break;
    move m = ai->next_move();
    played.push_back(to_uci(m));
    e.perform_move(m);
  }
  return played;
}

// Half moves of the test games: enough to see the AI's choices, few enough
// for searching AIs in the debug build.
constexpr int kPlies = 10;

TEST(AiRegistry, SameSeedPlaysSameGame) {
  for (const ai_info& info : available_ais()) {
    EXPECT_EQ(self_play(info, 5, kPlies), self_play(info, 5, kPlies))
        << info.name;
  }
}

TEST(AiRegistry, UsesRandomTellsWhetherTheSeedMatters) {
  // An AI that uses random numbers plays differently with another seed; one
  // that does not must ignore the seed (the GUI hides the seed for it).
  for (const ai_info& info : available_ais()) {
    if (info.uses_random) {
      EXPECT_NE(self_play(info, 1, kPlies), self_play(info, 2, kPlies))
          << info.name;
    } else {
      EXPECT_EQ(self_play(info, 1, kPlies), self_play(info, 2, kPlies))
          << info.name;
    }
  }
}

TEST(AiRegistry, NamesAreUnique) {
  const auto& ais = available_ais();
  for (size_t i = 0; i < ais.size(); ++i) {
    for (size_t j = i + 1; j < ais.size(); ++j) {
      EXPECT_NE(ais[i].name, ais[j].name);
    }
  }
}

TEST(AiRegistry, CreatedAiHasTheRegistryName) {
  engine e;
  e.new_game();
  for (const ai_info& info : available_ais()) {
    EXPECT_EQ(info.create(&e, 1)->get_name(), info.name);
  }
}

// AIs that search have a depth: a sensible range for the GUI, and the depth
// really reaches the AI (a deeper search looks at more positions, which a
// test can see in the number of moves it needs to find a mate).
TEST(AiRegistry, SearchingAisHaveADepthRange) {
  int with_depth = 0;
  for (const ai_info& info : available_ais()) {
    if (!info.create_with_depth) {
      EXPECT_EQ(info.default_depth, 0) << info.name;
      continue;
    }
    ++with_depth;
    EXPECT_GE(info.default_depth, 1) << info.name;
    EXPECT_LE(info.default_depth, info.max_depth_interactive) << info.name;
    EXPECT_LE(info.max_depth_interactive, info.max_depth) << info.name;
    engine e;
    e.new_game();
    std::unique_ptr<ai_template> ai = create_ai(info, &e, 1, 1);
    EXPECT_EQ(ai->get_name(), info.name);
    EXPECT_TRUE(has_move(e, to_uci(ai->next_move()))) << info.name;
  }
  EXPECT_EQ(with_depth, 2);  // Minimax and Alpha-Beta
}

// The depth given to create_ai() reaches the AI; 0 means its default.
TEST(AiRegistry, DepthReachesTheAi) {
  auto depth_of = [](ai_template* ai) {
    if (auto* m = dynamic_cast<minimax_ai*>(ai)) return m->depth();
    if (auto* a = dynamic_cast<alphabeta_ai*>(ai)) return a->depth();
    return -1;
  };
  for (const ai_info& info : available_ais()) {
    if (!info.create_with_depth) continue;
    engine e;
    e.new_game();
    EXPECT_EQ(depth_of(create_ai(info, &e, 1, 2).get()), 2) << info.name;
    EXPECT_EQ(depth_of(create_ai(info, &e, 1, 0).get()), info.default_depth)
        << info.name;
    EXPECT_EQ(depth_of(info.create(&e, 1).get()), info.default_depth)
        << info.name;
  }
}

}  // namespace
