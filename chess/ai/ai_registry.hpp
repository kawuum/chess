#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "template/ai_template.hpp"

/**
 * Describes one AI that a GUI can offer to the user.
 * To add a new AI, add one entry to the list in ai_registry.cpp.
 */
struct ai_info {
  std::string name;
  // Creates a new instance of this AI that plays on the given engine. The
  // seed starts its random generator: the same seed gives the same moves in
  // the same positions, so simulated games can be repeated exactly.
  std::function<std::unique_ptr<ai_template>(engine*, uint32_t seed)> create;
  // True if the AI uses random numbers at all (then the seed matters). A GUI
  // shows the seed only for such AIs.
  bool uses_random = false;
};

/**
 * Returns all available AIs. The list lives for the whole program, so
 * pointers to its entries stay valid.
 */
const std::vector<ai_info>& available_ais();
