#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "ai_registry.hpp"

/**
 * Games between two AIs without a board on screen, e.g. to compare two AIs
 * over many games. Everything here runs in the calling thread; a GUI that
 * wants to stay responsive calls play_match() from a thread of its own.
 */

/** How one game ended. */
struct game_record {
  game_result result = RUNNING;  // RUNNING: stopped at the half move limit
  draw_reason draw = NO_DRAW;    // why, if result is DRAW
  int plies = 0;                 // half moves played
};

/**
 * Plays one game from the start position. White's AI is created with seed,
 * Black's with seed + 1, so the same seed always gives the same game. The
 * game ends with checkmate, a draw, or after max_plies half moves.
 */
game_record play_game(const ai_info& white, const ai_info& black, uint32_t seed,
                      int max_plies);

/** Results of a match between a first and a second AI. */
struct match_stats {
  int games = 0;
  int first_wins = 0;
  int second_wins = 0;
  int draws = 0;
  int draws_by[5] = {};  // index: draw_reason (STALEMATE, ...)
  int unfinished = 0;    // stopped at the half move limit
  long long total_plies = 0;
};

struct match_settings {
  int games = 100;
  int max_plies = 500;  // a game that is still running then is unfinished
  uint32_t seed = 1;    // game i uses seed + 2 * i (see play_game)
};

/**
 * Plays settings.games games between first and second. The colors
 * alternate: first plays White in games 0, 2, 4, ... and Black in the
 * others. after_each_game (may be empty) is called after every game with
 * the results so far; if it returns false, the match stops early.
 */
match_stats play_match(
    const ai_info& first, const ai_info& second, const match_settings& settings,
    const std::function<bool(const match_stats&)>& after_each_game = nullptr);

/**
 * The results as a few lines of text, e.g. for a dialog. If both AIs have
 * the same name, they are called "(1)" and "(2)".
 */
std::string describe(const match_stats& stats, const std::string& first_ai,
                     const std::string& second_ai);
