// Color symmetry: White and Black must be treated exactly alike. Every
// position of many games is compared with its mirror image: the board
// flipped top to bottom, the colors swapped, the other side to move, and
// castling rights, en passant square and half move clock mirrored too.
// Move generation, check, the game result, the evaluation helpers, the
// naive AI's rating and the search evaluation (scoring) must then give
// mirrored answers. A rule that is only
// right for one color (e.g. a pawn direction or a castling square) shows
// up here.
#include <algorithm>
#include <string>
#include <vector>

#include "../ai/ai_registry.hpp"
#include "../ai/naive/naive_ai.hpp"
#include "../engine/evaluation.hpp"
#include "../engine/scoring.hpp"
#include "test_helpers.hpp"

namespace {

// FEN of the mirror image of the position in gh.
std::string mirrored_fen(const game_history& gh) {
  std::string fen;
  for (int y = 7; y >= 0; --y) {  // FEN starts with rank 8
    int empty = 0;
    for (int x = 0; x < 8; ++x) {
      piece p = gh.curr_board.get_piece(x, 7 - y);  // the mirrored rank
      if (!p.is_valid()) {
        ++empty;
        continue;
      }
      if (empty > 0) fen += static_cast<char>('0' + empty);
      empty = 0;
      char letter = "kqbnrp"[p.get_piece_type()];
      // Colors swapped: a black piece becomes white (upper case).
      if (p.get_piece_color() == piece_color::BLACK) letter -= 'a' - 'A';
      fen += letter;
    }
    if (empty > 0) fen += static_cast<char>('0' + empty);
    if (y > 0) fen += '/';
  }
  fen += gh.to_move == piece_color::WHITE ? " b " : " w ";

  // White's castling rights become Black's and the other way round.
  const int white = gh.castlingrights[static_cast<int>(piece_color::WHITE)];
  const int black = gh.castlingrights[static_cast<int>(piece_color::BLACK)];
  std::string castling;
  if (black & SHORT) castling += 'K';
  if (black & LONG) castling += 'Q';
  if (white & SHORT) castling += 'k';
  if (white & LONG) castling += 'q';
  fen += castling.empty() ? "-" : castling;

  // En passant square: same file, mirrored rank. (0, 0) means none.
  const int ex = std::get<0>(gh.ep_square);
  const int ey = std::get<1>(gh.ep_square);
  if (ex == 0 && ey == 0) {
    fen += " -";
  } else {
    fen += ' ';
    fen += static_cast<char>('a' + ex);
    fen += static_cast<char>('1' + 7 - ey);
  }
  fen += ' ' + std::to_string(gh.halfmove_clock) + " 1";
  return fen;
}

// "e2e4" -> "e7e5": the same move in the mirror image.
std::string mirrored_uci(std::string uci) {
  uci[1] = static_cast<char>('1' + '8' - uci[1]);
  uci[3] = static_cast<char>('1' + '8' - uci[3]);
  return uci;
}

game_result mirrored_result(game_result r) {
  if (r == WHITE_WIN) return BLACK_WIN;
  if (r == BLACK_WIN) return WHITE_WIN;
  return r;
}

const ai_info& random_ai_info() {
  for (const ai_info& info : available_ais()) {
    if (info.name == "Random AI") return info;
  }
  throw std::logic_error("no Random AI in the registry");
}

TEST(Symmetry, MirroredPositionsBehaveAlike) {
  const piece_color W = piece_color::WHITE;
  const piece_color B = piece_color::BLACK;
  naive_ai judge(nullptr, "judge", 1);  // only its evaluate() is used
  int positions = 0;
  // How often the tricky rules came up, to be sure they were compared.
  int castling = 0, en_passant = 0, promotion = 0;

  // Random games reach castling, en passant and promotions often enough.
  for (uint32_t game = 0; game < 40; ++game) {
    engine e;
    e.new_game();
    auto white = random_ai_info().create(&e, 100 + game);
    auto black = random_ai_info().create(&e, 900 + game);
    for (int ply = 0; ply < 200; ++ply) {
      game_history gh = e.get_current_gamestate();
      // A repetition needs the earlier positions, which the mirror image
      // (from a FEN) does not have.
      if (gh.draw == THREEFOLD_REPETITION) break;

      const std::string fen = mirrored_fen(gh);
      engine m;
      m.new_game(fen);
      game_history mh = m.get_current_gamestate();
      ++positions;

      ASSERT_EQ(mh.is_check, gh.is_check) << fen;
      ASSERT_EQ(mh.result, mirrored_result(gh.result)) << fen;
      ASSERT_EQ(mh.draw, gh.draw) << fen;

      std::vector<std::string> moves;
      for (const std::string& uci : legal_uci_moves(e)) {
        moves.push_back(mirrored_uci(uci));
      }
      std::vector<std::string> mirror_moves = legal_uci_moves(m);
      std::sort(moves.begin(), moves.end());
      std::sort(mirror_moves.begin(), mirror_moves.end());
      ASSERT_EQ(mirror_moves, moves) << fen;
      for (const move& mv : e.get_all_legal_moves(&gh)) {
        if (mv.type == CASTLING) ++castling;
        if (mv.type == ENPASSANT) ++en_passant;
        if (is_promotion(mv)) ++promotion;
      }

      const board& b = gh.curr_board;
      const board& mb = mh.curr_board;
      ASSERT_EQ(evaluation::material(mb, B), evaluation::material(b, W)) << fen;
      ASSERT_EQ(evaluation::attacked_squares(mb, B).get_num_bits_set(),
                evaluation::attacked_squares(b, W).get_num_bits_set())
          << fen;
      ASSERT_EQ(evaluation::threatened_value(mb, B),
                evaluation::threatened_value(b, W))
          << fen;
      ASSERT_EQ(evaluation::defended_pieces(mb, W).get_num_bits_set(),
                evaluation::defended_pieces(b, B).get_num_bits_set())
          << fen;
      ASSERT_EQ(judge.evaluate(mb, B), judge.evaluate(b, W)) << fen;
      ASSERT_EQ(scoring::score(mb, B), scoring::score(b, W)) << fen;

      if (gh.result != RUNNING) break;
      ai_template& ai = gh.to_move == W ? *white : *black;
      e.perform_move(ai.next_move());
    }
  }
  EXPECT_GT(positions, 3000);  // enough positions were compared
  EXPECT_GT(castling, 0);
  EXPECT_GT(en_passant, 0);
  EXPECT_GT(promotion, 0);
}

}  // namespace
