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
      {"Minimax AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<minimax_ai>(e, "Minimax AI", seed);
       },
       /*uses_random=*/true},
      // Same idea with alpha-beta and move ordering: 4 half moves deep.
      {"Alpha-Beta AI",
       [](engine* e, uint32_t seed) {
         return std::make_unique<alphabeta_ai>(e, "Alpha-Beta AI", seed);
       },
       /*uses_random=*/true},
  };
  return ais;
}
