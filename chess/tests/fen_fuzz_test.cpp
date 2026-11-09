// Fuzz tests for the FEN import: many random and damaged FEN strings. None
// may crash the engine (run them in the ASan build to also catch reads
// outside the board), and every FEN that is accepted must give a position
// the engine can play: all moves can be made and taken back.
#include <random>
#include <string>
#include <vector>

#include "../engine/move_generation.hpp"
#include "test_helpers.hpp"

namespace {

const char* const kValid[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2",
    "4k3/8/8/8/4P3/8/8/4K3 b - e3 0 1",
};

// Characters that occur in FEN strings, and a few that do not.
const std::string kAlphabet = "KQRBNPkqrbnp12345678/ wb-acdefgh09x\t";

// An accepted position must be playable: the side not to move is not in
// check, every legal move can be made and taken back, and a small perft
// runs. Returns the number of legal moves.
size_t check_playable(engine& e, const std::string& fen) {
  const game_history& state = e.current_state();
  board b = state.curr_board;
  const piece_color waiting = state.to_move == piece_color::WHITE
                                  ? piece_color::BLACK
                                  : piece_color::WHITE;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const piece p = b.get_piece(x, y);
      if (p.is_valid() && p.get_piece_type() == KING &&
          p.get_piece_color() == waiting) {
        EXPECT_FALSE(
            move_generation::is_square_attacked(b, x, y, state.to_move))
            << fen;
      }
    }
  }
  const std::vector<move> moves = e.get_all_legal_moves();
  for (const move& m : moves) {
    e.perform_move(m);
    e.undo_move();
  }
  EXPECT_EQ(e.get_all_legal_moves(), moves) << fen;
  perft(e, 2);
  return moves.size();
}

TEST(FenFuzz, DamagedFensAreRejectedOrPlayable) {
  std::mt19937 rng(2026);
  int accepted = 0, rejected = 0;
  for (int i = 0; i < 10000; ++i) {
    std::string fen = kValid[i % 6];
    const int changes = 1 + static_cast<int>(rng() % 3);
    for (int c = 0; c < changes && !fen.empty(); ++c) {
      const size_t pos = rng() % fen.size();
      const char ch = kAlphabet[rng() % kAlphabet.size()];
      switch (rng() % 3) {
        case 0:
          fen[pos] = ch;
          break;
        case 1:
          fen.insert(fen.begin() + pos, ch);
          break;
        default:
          fen.erase(pos, 1);
      }
    }
    engine e;
    if (e.try_new_game(fen) == fen_error::NONE) {
      ++accepted;
      check_playable(e, fen);
    } else {
      ++rejected;
      // A rejected FEN leaves the engine without a game; a new one works.
      e.new_game();
      EXPECT_EQ(e.get_all_legal_moves().size(), 20u);
    }
  }
  // Both outcomes must really occur, or the test proves nothing.
  EXPECT_GT(accepted, 200);
  EXPECT_GT(rejected, 5000);
}

// Random positions with a valid structure: one king per color on random
// squares, other pieces on about every 5th square (pawns on rank 1 or 8
// now and then), random side to move, sometimes castling rights and an en
// passant square. Many are rejected by the position checks (king in check,
// rights without pieces); those that pass must be playable.
TEST(FenFuzz, RandomPositionsAreRejectedOrPlayable) {
  std::mt19937 rng(7);
  const std::string others = "QRBNPqrbnp";
  int accepted = 0;
  size_t total_moves = 0;
  for (int i = 0; i < 3000; ++i) {
    char square[64];  // index 8 * row + file, row 0 = rank 8 (FEN order)
    for (char& c : square) c = '.';
    const int white_king = static_cast<int>(rng() % 64);
    int black_king = static_cast<int>(rng() % 63);
    if (black_king >= white_king) ++black_king;
    square[white_king] = 'K';
    square[black_king] = 'k';
    for (int i_sq = 0; i_sq < 64; ++i_sq) {
      if (square[i_sq] != '.' || rng() % 5 != 0) continue;
      const char p = others[rng() % others.size()];
      const bool back_rank = i_sq < 8 || i_sq >= 56;
      if ((p == 'P' || p == 'p') && back_rank && rng() % 10 != 0) continue;
      square[i_sq] = p;
    }
    std::string fen;
    for (int row = 0; row < 8; ++row) {
      int empty = 0;
      for (int file = 0; file < 8; ++file) {
        const char c = square[8 * row + file];
        if (c == '.') {
          ++empty;
          continue;
        }
        if (empty > 0) fen += static_cast<char>('0' + empty);
        empty = 0;
        fen += c;
      }
      if (empty > 0) fen += static_cast<char>('0' + empty);
      if (row < 7) fen += '/';
    }
    fen += rng() % 2 ? " w " : " b ";
    std::string castling;
    for (char c : std::string("KQkq")) {
      if (rng() % 10 == 0) castling += c;
    }
    fen += castling.empty() ? "-" : castling;
    if (rng() % 10 == 0) {
      fen += ' ';
      fen += static_cast<char>('a' + rng() % 8);
      fen += rng() % 2 ? '3' : '6';
    } else {
      fen += " -";
    }
    fen += " " + std::to_string(rng() % 120) + " " +
           std::to_string(1 + rng() % 200);

    engine e;
    if (e.try_new_game(fen) == fen_error::NONE) {
      ++accepted;
      total_moves += check_playable(e, fen);
    }
  }
  EXPECT_GT(accepted, 500);
  EXPECT_GT(total_moves, 5000u);
}

}  // namespace
