#include "ai_registry.hpp"

#include "naive/naive_ai.hpp"
#include "random/random_ai.hpp"

const std::vector<ai_info>& available_ais() {
  // Built once on first use (thread-safe since C++11).
  static const std::vector<ai_info> ais = {
      {"Random AI", [](engine* e) { return std::make_unique<random_ai>(e); }},
      {"Naive AI", [](engine* e) { return std::make_unique<naive_ai>(e); }},
  };
  return ais;
}
