#pragma once

#include <string>

#include "engine.hpp"
#include "move.hpp"

/**
 * The move m in standard algebraic notation (SAN), as in chess books and
 * move lists: "e4", "Nf3", "exd5", "O-O", "Nbd2", "e8=Q+", "Qf8#". Piece
 * letters are English: K, Q, R, B, N; pawns have none.
 *
 * m must be legal in the engine's current position. To find out whether the
 * move gives check or mate, the move is played on the engine and taken back;
 * afterwards the position is the same as before.
 */
std::string to_san(engine& e, const move& m);
