#include "ai_registry.hpp"

#include "alphabeta/alphabeta_ai.hpp"
#include "minimax/minimax_ai.hpp"
#include "naive/naive_ai.hpp"
#include "random/random_ai.hpp"

const std::vector<ai_info>& available_ais() {
  // Built once on first use (thread-safe since C++11).
  static const std::vector<ai_info> ais = {
      // Picks every move at random.
      {"Random AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<random_ai>(e, "Random AI", seed);
       },
       /*uses_random=*/true},
      // Picks at random among equally good moves.
      {"Naive AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<naive_ai>(e, "Naive AI", seed);
       },
       /*uses_random=*/true},
      // Looks 3 half moves ahead; at random among equally good moves.
      // Measured on Kiwipete: depth 3 0.08 s, depth 4 3.5 s per move.
      {"Minimax AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<minimax_ai>(e, "Minimax AI", seed);
       },
       /*uses_random=*/true,
       [](engine* e, uint32_t seed, int depth) {
         return std::make_unique<minimax_ai>(e, "Minimax AI", seed, depth);
       },
       minimax_ai::kDefaultDepth, /*max_depth_interactive=*/3,
       /*max_depth=*/4},
      // Same idea with alpha-beta and move ordering: 4 half moves deep.
      // Measured over game positions: depth 5 0.17 s per move (at most
      // 0.5 s), depth 6 1.2 s (at most 4.5 s), so the GUI stops at 5.
      {"Alpha-Beta AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<alphabeta_ai>(e, "Alpha-Beta AI", seed);
       },
       /*uses_random=*/true,
       [](engine* e, uint32_t seed, int depth) {
         return std::make_unique<alphabeta_ai>(e, "Alpha-Beta AI", seed, depth);
       },
       alphabeta_ai::kDefaultDepth, /*max_depth_interactive=*/5,
       /*max_depth=*/8},
  };
  return ais;
}

std::unique_ptr<ai_template> create_ai(const ai_info& info, engine* e,
                                       uint32_t seed, int depth) {
  if (depth > 0 && info.create_with_depth) {
    return info.create_with_depth(e, seed, depth);
  }
  return info.create(e, seed);
}
