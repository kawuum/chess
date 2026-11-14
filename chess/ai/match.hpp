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
 * Plays one game from the start position. Each AI gets its own seed, so the
 * same two seeds always give the same game. The game ends with checkmate, a
 * draw, or after max_plies half moves. The depths go to AIs that search
 * (0 = their default, see create_ai()).
 */
game_record play_game(const ai_info& white, const ai_info& black,
                      uint32_t white_seed, uint32_t black_seed, int max_plies,
                      int white_depth = 0, int black_depth = 0);

/** Results of a match between a first and a second AI. */
struct match_stats {
  int games = 0;
  int first_wins = 0;
  int second_wins = 0;
  int white_wins = 0;  // the same wins, counted by color
  int black_wins = 0;
  int draws = 0;
  int draws_by[5] = {};  // index: draw_reason (STALEMATE, ...)
  int unfinished = 0;    // stopped at the half move limit
  long long total_plies = 0;
};

struct match_settings {
  int games = 100;
  int max_plies = 500;  // a game that is still running then is unfinished
  // Seeds of the two AIs. In game i the first AI plays with first_seed + i,
  // the second with second_seed + i: every game is different, and the same
  // seeds repeat the whole match.
  uint32_t first_seed = 1;
  uint32_t second_seed = 2;
  // Search depths for AIs that have one; 0 = the AI's default depth.
  int first_depth = 0;
  int second_depth = 0;
  // false: the first AI always plays White. true: the colors swap after
  // every game, so White's advantage of the first move is shared fairly.
  bool swap_colors = true;
};

/**
 * Plays settings.games games between first and second. The first AI plays
 * White in game 0; with swap_colors it plays Black in games 1, 3, 5, ...
 * after_each_game (may be empty) is called after every game with the
 * results so far; if it returns false, the match stops early.
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
