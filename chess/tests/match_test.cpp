// Tests for games between two AIs without a board (ai/match.hpp).
#include "../ai/match.hpp"

#include <cstdlib>
#include <string>
#include <vector>

#include "test_helpers.hpp"

namespace {

const ai_info& ai_named(const std::string& name) {
  for (const ai_info& info : available_ais()) {
    if (info.name == name) return info;
  }
  throw std::invalid_argument("no AI named " + name);
}

const ai_info& random_ai_info() { return ai_named("Random AI"); }
const ai_info& naive_ai_info() { return ai_named("Naive AI"); }

// Regression test with exact numbers: a simulation run in the GUI on
// 2026-11-05 (Naive AI against Naive AI, fixed colors, these seeds). The
// games are fully determined by the seeds, so every number must come out
// exactly. It fails on any change to the engine, the draw rules, the naive
// AI or the order of the generated moves; after an intended change the new
// numbers are entered here on purpose (like the memory baseline).
// Takes about 13 s in the release build, so it only runs with
// CHESS_SLOW_TESTS=1 (together with the deep perft tests: ctest -R Deep).
TEST(DeepMatch, NaiveAgainstNaiveExactNumbers) {
  if (std::getenv("CHESS_SLOW_TESTS") == nullptr) {
    GTEST_SKIP() << "set CHESS_SLOW_TESTS=1 to run the long match";
  }
  match_settings settings;
  settings.games = 1452;
  settings.max_plies = 500;
  settings.first_seed = 444990164;  // White
  settings.second_seed = 57235226;  // Black
  settings.swap_colors = false;
  match_stats s = play_match(naive_ai_info(), naive_ai_info(), settings);
  EXPECT_EQ(s.games, 1452);
  EXPECT_EQ(s.first_wins, 120);   // White
  EXPECT_EQ(s.second_wins, 282);  // Black: moving second helps the naive AI
  EXPECT_EQ(s.white_wins, 120);   // fixed colors: first AI = White
  EXPECT_EQ(s.black_wins, 282);
  EXPECT_EQ(s.draws, 1050);
  EXPECT_EQ(s.draws_by[STALEMATE], 0);
  EXPECT_EQ(s.draws_by[THREEFOLD_REPETITION], 963);
  EXPECT_EQ(s.draws_by[FIFTY_MOVE_RULE], 67);
  EXPECT_EQ(s.draws_by[INSUFFICIENT_MATERIAL], 20);
  EXPECT_EQ(s.unfinished, 0);
  // 112.99 half moves per game; describe() rounds down and shows 112.
  EXPECT_EQ(s.total_plies, 164073);
}

TEST(Match, GameEndsWithAResult) {
  game_record r = play_game(random_ai_info(), random_ai_info(), 1, 2, 2000);
  EXPECT_NE(r.result, RUNNING);  // with the draw rules every game ends
  EXPECT_GT(r.plies, 0);
}

TEST(Match, SameSeedsPlaySameGame) {
  game_record a = play_game(naive_ai_info(), random_ai_info(), 7, 8, 500);
  game_record b = play_game(naive_ai_info(), random_ai_info(), 7, 8, 500);
  EXPECT_EQ(a.result, b.result);
  EXPECT_EQ(a.draw, b.draw);
  EXPECT_EQ(a.plies, b.plies);
}

TEST(Match, HalfMoveLimitStopsTheGame) {
  game_record r = play_game(random_ai_info(), random_ai_info(), 1, 2, 10);
  EXPECT_EQ(r.result, RUNNING);
  EXPECT_EQ(r.plies, 10);
}

TEST(Match, StatsAddUp) {
  match_settings settings;
  settings.games = 20;
  match_stats s = play_match(random_ai_info(), random_ai_info(), settings);
  EXPECT_EQ(s.games, 20);
  EXPECT_EQ(s.first_wins + s.second_wins + s.draws + s.unfinished, 20);
  EXPECT_EQ(s.white_wins + s.black_wins, s.first_wins + s.second_wins);
  int by_reason = 0;
  for (int count : s.draws_by) by_reason += count;
  EXPECT_EQ(by_reason, s.draws);
  EXPECT_EQ(s.draws_by[NO_DRAW], 0);
  EXPECT_GT(s.total_plies, 0);
}

TEST(Match, ColorsAlternateAndEachAiHasItsOwnSeed) {
  // Two AIs that note their creation with the seed they got. play_game()
  // creates White's AI before Black's, so the order shows the colors.
  std::vector<std::string> created;
  auto noting = [&created](const std::string& name) {
    return ai_info{name, [&created, name](engine* e, uint32_t seed) {
                     created.push_back(name + " " + std::to_string(seed));
                     return random_ai_info().create(e, seed);
                   }};
  };
  ai_info first = noting("first");
  ai_info second = noting("second");
  match_settings settings;
  settings.games = 3;
  settings.max_plies = 2;
  settings.first_seed = 10;
  settings.second_seed = 20;
  play_match(first, second, settings);
  EXPECT_EQ(
      created, (std::vector<std::string>{"first 10", "second 20",   // game 0
                                         "second 21", "first 11",   // game 1
                                         "first 12", "second 22"})  // game 2
  );

  // Without swapping, the first AI is White in every game.
  created.clear();
  settings.swap_colors = false;
  play_match(first, second, settings);
  EXPECT_EQ(
      created, (std::vector<std::string>{"first 10", "second 20",   // game 0
                                         "first 11", "second 21",   // game 1
                                         "first 12", "second 22"})  // game 2
  );
}

TEST(Match, SameSeedsRepeatTheMatch) {
  match_settings settings;
  settings.games = 6;
  settings.first_seed = 100;
  settings.second_seed = 200;
  match_stats a = play_match(random_ai_info(), random_ai_info(), settings);
  match_stats b = play_match(random_ai_info(), random_ai_info(), settings);
  EXPECT_EQ(a.total_plies, b.total_plies);
  EXPECT_EQ(a.first_wins, b.first_wins);
  // Another seed for only the second AI gives other games.
  settings.second_seed = 201;
  match_stats c = play_match(random_ai_info(), random_ai_info(), settings);
  EXPECT_NE(a.total_plies, c.total_plies);
}

TEST(Match, WinsAreCountedForTheRightAi) {
  // The naive AI beats the random AI almost always, whatever its color.
  match_settings settings;
  settings.games = 10;
  match_stats s = play_match(naive_ai_info(), random_ai_info(), settings);
  EXPECT_GE(s.first_wins, 8);
  EXPECT_EQ(s.second_wins, 0);
  match_stats swapped = play_match(random_ai_info(), naive_ai_info(), settings);
  EXPECT_GE(swapped.second_wins, 8);
  EXPECT_EQ(swapped.first_wins, 0);
}

TEST(Match, CallbackCanStopTheMatch) {
  match_settings settings;
  settings.games = 50;
  int calls = 0;
  match_stats s = play_match(random_ai_info(), random_ai_info(), settings,
                             [&calls](const match_stats& so_far) {
                               ++calls;
                               EXPECT_EQ(so_far.games, calls);
                               return so_far.games < 3;  // stop after 3
                             });
  EXPECT_EQ(s.games, 3);
  EXPECT_EQ(calls, 3);
}

TEST(Match, Describe) {
  match_stats s;
  s.games = 4;
  s.first_wins = 3;
  s.draws = 1;
  s.draws_by[THREEFOLD_REPETITION] = 1;
  s.total_plies = 400;
  std::string text = describe(s, "Naive AI", "Random AI");
  EXPECT_NE(text.find("4 games: Naive AI against Random AI"), std::string::npos)
      << text;
  EXPECT_NE(text.find("Naive AI wins: 3 (75%)"), std::string::npos) << text;
  EXPECT_NE(text.find("Random AI wins: 0 (0%)"), std::string::npos) << text;
  EXPECT_NE(text.find("repetition 1"), std::string::npos) << text;
  EXPECT_NE(text.find("Average length: 100 half moves"), std::string::npos)
      << text;
}

TEST(Match, DescribeAnAiAgainstItself) {
  match_stats s;
  s.games = 2;
  s.first_wins = 2;
  std::string text = describe(s, "Random AI", "Random AI");
  EXPECT_NE(text.find("Random AI (1) wins: 2"), std::string::npos) << text;
  EXPECT_NE(text.find("Random AI (2) wins: 0"), std::string::npos) << text;
}

}  // namespace
