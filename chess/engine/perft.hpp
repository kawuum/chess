#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "engine.hpp"

/**
 * Perft ("performance test"): counts all legal move sequences of the given
 * length from the engine's current position. The results for well-known
 * positions are published, so comparing against them finds bugs in the move
 * generator. A promotion counts once per piece it can promote to (4 times).
 *
 * The engine is left in the same position as before the call.
 * @param e The engine with the position to count from
 * @param depth Number of half moves
 * @returns Number of leaf positions
 */
uint64_t perft(engine& e, int depth);

/**
 * Like perft(), but split by the first move: for each legal move the number of
 * leaf positions below it. Compared with a reference engine this shows which
 * move is counted wrong.
 * @returns Pairs of (move in UCI notation such as "e2e4" or "e7e8q", count)
 */
std::vector<std::pair<std::string, uint64_t>> perft_divide(engine& e,
                                                           int depth);

/**
 * Formats a move in UCI notation: start and target square, e.g. "e2e4",
 * plus the promotion piece for promotions ("e7e8q").
 */
std::string to_uci(const move& m, piece_type promotion = QUEEN);
