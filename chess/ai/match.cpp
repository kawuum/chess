#include "match.hpp"

#include <memory>
#include <sstream>

namespace {

// "94 (94%)": a count and its share of all games.
std::string with_percent(int count, int games) {
  std::ostringstream text;
  text << count;
  if (games > 0) text << " (" << (100 * count + games / 2) / games << "%)";
  return text.str();
}

}  // namespace

game_record play_game(const ai_info& white, const ai_info& black,
                      uint32_t white_seed, uint32_t black_seed, int max_plies) {
  engine e;
  e.new_game();
  std::unique_ptr<ai_template> white_ai = white.create(&e, white_seed);
  std::unique_ptr<ai_template> black_ai = black.create(&e, black_seed);

  game_record record;
  while (e.get_current_gamestate().result == RUNNING &&
         record.plies < max_plies) {
    ai_template& ai =
        e.get_color_to_move() == piece_color::WHITE ? *white_ai : *black_ai;
    e.perform_move(ai.next_move());
    ++record.plies;
  }
  record.result = e.get_current_gamestate().result;
  record.draw = e.get_current_gamestate().draw;
  return record;
}

match_stats play_match(
    const ai_info& first, const ai_info& second, const match_settings& settings,
    const std::function<bool(const match_stats&)>& after_each_game) {
  match_stats stats;
  for (int game = 0; game < settings.games; ++game) {
    const bool first_is_white = game % 2 == 0;
    const ai_info& white = first_is_white ? first : second;
    const ai_info& black = first_is_white ? second : first;
    const uint32_t first_seed = settings.first_seed + game;
    const uint32_t second_seed = settings.second_seed + game;
    const uint32_t white_seed = first_is_white ? first_seed : second_seed;
    const uint32_t black_seed = first_is_white ? second_seed : first_seed;
    game_record record =
        play_game(white, black, white_seed, black_seed, settings.max_plies);

    ++stats.games;
    stats.total_plies += record.plies;
    if (record.result == DRAW) {
      ++stats.draws;
      ++stats.draws_by[record.draw];
    } else if (record.result == RUNNING) {
      ++stats.unfinished;
    } else {
      const bool white_won = record.result == WHITE_WIN;
      if (white_won == first_is_white) {
        ++stats.first_wins;
      } else {
        ++stats.second_wins;
      }
    }

    if (after_each_game && !after_each_game(stats)) break;  // cancelled
  }
  return stats;
}

std::string describe(const match_stats& stats, const std::string& first_ai,
                     const std::string& second_ai) {
  // An AI against itself: number the two, so the lines can be told apart.
  std::string first_name = first_ai;
  std::string second_name = second_ai;
  if (first_name == second_name) {
    first_name += " (1)";
    second_name += " (2)";
  }
  std::ostringstream text;
  text << stats.games << " games: " << first_name << " against " << second_name
       << "\n";
  text << first_name << " wins: " << with_percent(stats.first_wins, stats.games)
       << "\n";
  text << second_name
       << " wins: " << with_percent(stats.second_wins, stats.games) << "\n";
  text << "Draws: " << with_percent(stats.draws, stats.games) << " - stalemate "
       << stats.draws_by[STALEMATE] << ", repetition "
       << stats.draws_by[THREEFOLD_REPETITION] << ", 50 moves "
       << stats.draws_by[FIFTY_MOVE_RULE] << ", material "
       << stats.draws_by[INSUFFICIENT_MATERIAL] << "\n";
  text << "Unfinished (half move limit): " << stats.unfinished << "\n";
  if (stats.games > 0) {
    text << "Average length: " << stats.total_plies / stats.games
         << " half moves\n";
  }
  return text.str();
}
