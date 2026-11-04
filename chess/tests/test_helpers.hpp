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

// Plays a move given in UCI notation. Fails the test if it is not legal.
inline void play(engine& e, const std::string& uci) {
  game_history gs = e.get_current_gamestate();
  for (move m : e.get_all_legal_moves(&gs)) {
    if (to_uci(m) == uci.substr(0, 4) || to_uci(m) == uci) {
      if (m.type == PROMOTION || m.type == CAPTURING_PROMOTION) {
        piece q(QUEEN, e.get_color_to_move());
        e.perform_move(m, q);
      } else {
        e.perform_move(m);
      }
      return;
    }
  }
  ADD_FAILURE() << "move " << uci << " is not legal";
}
