// engine::perform_move() only accepts legal moves. Anything else throws
// std::invalid_argument and leaves the position unchanged.
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "../engine/move_generation.hpp"
#include "test_helpers.hpp"

namespace {

const piece kWhitePawn(PAWN, piece_color::WHITE);

// A move built by hand, as a buggy caller might do it.
move make_move(piece mover, int fx, int fy, int tx, int ty, move_type type) {
  return move{mover,
              static_cast<uint8_t>(fx),
              static_cast<uint8_t>(fy),
              static_cast<uint8_t>(tx),
              static_cast<uint8_t>(ty),
              type};
}

void expect_start_position(engine& e) {
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 0);
  EXPECT_EQ(e.get_color_to_move(), piece_color::WHITE);
  EXPECT_EQ(legal_uci_moves(e).size(), 20u);
  EXPECT_EQ(e.get_current_board().get_piece(4, 1).get_piece_type(), PAWN);
}

TEST(MoveValidation, IllegalTargetThrowsAndKeepsPosition) {
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 4, MOVE);  // e2-e5
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, OpponentPieceThrows) {
  engine e;
  e.new_game();
  move m = make_move(piece(PAWN, piece_color::BLACK), 4, 6, 4, 4, MOVE);
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, EmptySquareThrows) {
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 3, 4, 4, MOVE);  // e4-e5, e4 is empty
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  expect_start_position(e);
}

TEST(MoveValidation, NoMovesAfterGameOver) {
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  move m = make_move(kWhitePawn, 0, 1, 0, 2, MOVE);  // a2-a3 after mate
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
}

TEST(MoveValidation, UsesTheEnginesMoveData) {
  // The caller claims e2-e4 is a capture. The engine uses its own move list,
  // so the wrong type does no harm.
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 3, CAPTURE);
  e.perform_move(m);
  board& b = e.get_current_board();
  EXPECT_EQ(b.get_piece(4, 3).get_piece_type(), PAWN);
  EXPECT_FALSE(b.get_piece(4, 1).is_valid());
  EXPECT_EQ(b.get_num_pieces(), 32);
}

// --- Promotion --------------------------------------------------------------

const char* const kPromotionFen = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1";

// A promotion move of the pawn on a7, built by hand: the target square a8 is
// right, the promotion piece is whatever the test sets.
move promotion_move(piece_type promotion) {
  move m = make_move(kWhitePawn, 0, 6, 0, 7, PROMOTION);
  m.promotion = promotion;
  return m;
}

TEST(MoveValidation, FourMovesPerPromotion) {
  // Every promotion piece is its own legal move.
  engine e;
  e.new_game(kPromotionFen);
  std::vector<std::string> moves;
  for (const move& m : e.get_legal_moves(0, 6)) moves.push_back(to_uci(m));
  std::sort(moves.begin(), moves.end());
  EXPECT_EQ(moves,
            (std::vector<std::string>{"a7a8b", "a7a8n", "a7a8q", "a7a8r"}));
}

TEST(MoveValidation, PromotionWithoutPieceThrows) {
  // move::promotion defaults to KING, which means "none".
  engine e;
  e.new_game(kPromotionFen);
  move m = make_move(kWhitePawn, 0, 6, 0, 7, PROMOTION);
  EXPECT_THROW(e.perform_move(m), std::invalid_argument);
  EXPECT_EQ(e.get_current_board().get_piece(0, 6).get_piece_type(), PAWN);
}

TEST(MoveValidation, PromotionToKingOrPawnThrows) {
  engine e;
  e.new_game(kPromotionFen);
  EXPECT_THROW(e.perform_move(promotion_move(KING)), std::invalid_argument);
  EXPECT_THROW(e.perform_move(promotion_move(PAWN)), std::invalid_argument);
  EXPECT_EQ(e.get_current_board().get_piece(0, 6).get_piece_type(), PAWN);
}

TEST(MoveValidation, PromotionFieldIsIgnoredForNormalMoves) {
  // The promotion piece only means something for promotions.
  engine e;
  e.new_game();
  move m = make_move(kWhitePawn, 4, 1, 4, 3, MOVE);
  m.promotion = QUEEN;
  e.perform_move(m);
  EXPECT_EQ(e.get_current_board().get_piece(4, 3).get_piece_type(), PAWN);
}

TEST(MoveValidation, UnderpromotionToKnight) {
  engine e;
  e.new_game(kPromotionFen);
  e.perform_move(promotion_move(KNIGHT));
  piece p = e.get_current_board().get_piece(0, 7);  // a8
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), KNIGHT);
  EXPECT_EQ(p.get_piece_color(), piece_color::WHITE);
}

// The cheap getters for the search (no copy of the game state, moves from
// the cache) must give the same as the copying ones, also after undo_move().
TEST(MoveValidation, CheapGettersMatchTheCopyingOnes) {
  engine e;
  e.new_game();
  std::mt19937 rng(5);
  for (int ply = 0; ply < 120; ++ply) {
    game_history copy = e.get_current_gamestate();
    const game_history& state = e.current_state();
    ASSERT_TRUE(state.curr_board == copy.curr_board);
    ASSERT_EQ(state.num_halfmoves, copy.num_halfmoves);
    ASSERT_EQ(state.result, copy.result);
    const std::vector<move> cached = e.get_all_legal_moves();
    ASSERT_EQ(cached, e.get_all_legal_moves(&copy)) << "ply " << ply;
    if (cached.empty()) break;
    // The reference stays valid while later moves are played and undone.
    e.perform_move(cached[rng() % cached.size()]);
    const std::vector<move> replies = e.get_all_legal_moves();
    if (!replies.empty()) {
      e.perform_move(replies[0]);
      e.undo_move();
    }
    e.undo_move();
    ASSERT_EQ(state.num_halfmoves, copy.num_halfmoves);
    ASSERT_EQ(e.get_all_legal_moves(), cached);
    e.perform_move(cached[rng() % cached.size()]);
  }
}

// perform_move() only looks for one legal move to find checkmate and
// stalemate. Check and result must be exactly what the full move list says,
// in random games (which end in mates, stalemates and draws) and in a few
// set positions.
TEST(MoveValidation, ResultAfterMoveMatchesFullGeneration) {
  auto check = [](engine& e, const std::string& where) {
    game_history full = e.get_current_gamestate();
    full.result = RUNNING;
    full.draw = NO_DRAW;
    full.is_check = !full.is_check;  // must be set again
    move_generation mg;
    const std::vector<move> moves = mg.generate_all_moves(&full);
    const game_history& state = e.current_state();
    EXPECT_EQ(state.is_check, full.is_check) << where;
    if (state.result == DRAW && state.draw != STALEMATE) {
      EXPECT_FALSE(moves.empty()) << where;  // a draw rule ended the game
    } else {
      EXPECT_EQ(state.result, full.result) << where;
      EXPECT_EQ(state.draw, full.draw) << where;
    }
  };
  int mates = 0, stalemates = 0;
  for (uint32_t seed = 1; seed <= 60; ++seed) {
    engine e;
    e.new_game();
    std::mt19937 rng(seed);
    for (int ply = 0; ply < 400; ++ply) {
      const std::vector<move> moves = e.get_all_legal_moves();
      if (moves.empty()) break;
      e.perform_move(moves[rng() % moves.size()]);
      check(e, "seed " + std::to_string(seed) + " ply " + std::to_string(ply));
    }
    const game_result r = e.current_state().result;
    if (r == WHITE_WIN || r == BLACK_WIN) ++mates;
    if (r == DRAW && e.current_state().draw == STALEMATE) ++stalemates;
  }
  // Mates in one move, stalemate in one move: the last move ends the game.
  const char* const endings[][2] = {
      {"7k/8/6K1/8/8/8/8/5Q2 w - - 0 1", "f1f8"},  // mate
      {"7k/8/5K2/8/8/8/8/6Q1 w - - 0 1", "g1g6"},  // stalemate
      {"k7/8/1K6/8/8/8/8/7R w - - 0 1", "h1h8"},   // mate
  };
  for (const auto& ending : endings) {
    engine e;
    e.new_game(ending[0]);
    play(e, ending[1]);
    check(e, ending[0]);
    const game_result r = e.current_state().result;
    if (r == WHITE_WIN || r == BLACK_WIN) ++mates;
    if (r == DRAW && e.current_state().draw == STALEMATE) ++stalemates;
  }
  // The test must really have seen both endings.
  EXPECT_GE(mates, 2);
  EXPECT_GE(stalemates, 1);
}

}  // namespace
