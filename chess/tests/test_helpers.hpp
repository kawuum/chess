#pragma once

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../engine/engine.hpp"
#include "../engine/perft.hpp"

// All legal moves of the current position in UCI notation ("e2e4").
inline std::vector<std::string> legal_uci_moves(engine& e) {
  game_history gs = e.get_current_gamestate();
  std::vector<std::string> result;
  for (const move& m : e.get_all_legal_moves(&gs)) result.push_back(to_uci(m));
  return result;
}

inline bool has_move(engine& e, const std::string& uci) {
  for (const std::string& m : legal_uci_moves(e)) {
    if (m == uci) return true;
  }
  return false;
}

// Plays a move given in UCI notation, e.g. "e2e4" or "a7a8n" for a
// promotion to a knight. Fails the test if it is not legal.
inline void play(engine& e, const std::string& uci) {
  game_history gs = e.get_current_gamestate();
  for (const move& m : e.get_all_legal_moves(&gs)) {
    if (to_uci(m) == uci) {
      e.perform_move(m);
      return;
    }
  }
  ADD_FAILURE() << "move " << uci << " is not legal";
}
