// Tests for engine::new_game(fen): parsing of all FEN fields, error handling
// and the game state of loaded positions.
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "test_helpers.hpp"

namespace {

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

TEST(Fen, StartPositionEqualsNewGame) {
  engine from_fen, standard;
  from_fen.new_game(kStart);
  standard.new_game();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      piece a = from_fen.get_current_board().get_piece(x, y);
      piece b = standard.get_current_board().get_piece(x, y);
      ASSERT_EQ(a.is_valid(), b.is_valid()) << "square " << x << "," << y;
      if (a.is_valid()) {
        EXPECT_EQ(a.get_piece_type(), b.get_piece_type());
        EXPECT_EQ(a.get_piece_color(), b.get_piece_color());
      }
    }
  }
  EXPECT_EQ(legal_uci_moves(standard).size(), 20u);
}

TEST(Fen, PiecePlacement) {
  engine e;
  e.new_game("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
  board& b = e.get_current_board();
  EXPECT_EQ(b.get_num_pieces(), 4);
  piece rook = b.get_piece(0, 0);  // a1
  ASSERT_TRUE(rook.is_valid());
  EXPECT_EQ(rook.get_piece_type(), ROOK);
  EXPECT_EQ(rook.get_piece_color(), piece_color::WHITE);
  piece king = b.get_piece(4, 7);  // e8
  ASSERT_TRUE(king.is_valid());
  EXPECT_EQ(king.get_piece_type(), KING);
  EXPECT_EQ(king.get_piece_color(), piece_color::BLACK);
  EXPECT_FALSE(b.get_piece(4, 4).is_valid());  // e5 is empty
}

TEST(Fen, SideToMoveCastlingAndMoveNumber) {
  engine e;
  e.new_game("r3k2r/8/8/8/8/8/8/R3K2R b Kq - 3 8");
  game_history gs = e.get_current_gamestate();
  EXPECT_EQ(gs.to_move, piece_color::BLACK);
  EXPECT_EQ(gs.castlingrights[(int)piece_color::WHITE], SHORT);
  EXPECT_EQ(gs.castlingrights[(int)piece_color::BLACK], LONG);
  EXPECT_EQ(gs.num_halfmoves, 15);  // move 8, black to move
}

TEST(Fen, EnPassantSquare) {
  engine e;
  e.new_game("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2");
  game_history gs = e.get_current_gamestate();
  EXPECT_EQ(std::get<0>(gs.ep_square), 3);  // d
  EXPECT_EQ(std::get<1>(gs.ep_square), 5);  // 6
  EXPECT_TRUE(has_move(e, "e5d6"));         // the en passant capture
}

TEST(Fen, MoveCountersAreOptional) {
  engine e;
  EXPECT_NO_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w - -"));
}

TEST(Fen, InvalidInputThrows) {
  engine e;
  EXPECT_THROW(e.new_game(""), std::invalid_argument);
  EXPECT_THROW(e.new_game("nonsense"), std::invalid_argument);
  // only 7 ranks
  EXPECT_THROW(e.new_game("8/8/8/8/8/8/4K3 w - - 0 1"), std::invalid_argument);
  // 9 squares in a rank
  EXPECT_THROW(e.new_game("4k4/8/8/8/8/8/8/4K3 w - - 0 1"),
               std::invalid_argument);
  // unknown piece letter
  EXPECT_THROW(e.new_game("4x3/8/8/8/8/8/8/4K3 w - - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 x - - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w KX - 0 1"),
               std::invalid_argument);
  EXPECT_THROW(e.new_game("4k3/8/8/8/8/8/8/4K3 w - z9 0 1"),
               std::invalid_argument);
}

struct Rejected {
  const char* fen;
  fen_error error;
};

const Rejected kRejected[] = {
    {"", fen_error::FIELDS},
    {"nonsense", fen_error::FIELDS},
    {"4k3/8/8/8/8/8/8/4K3 w - - 0 1 extra", fen_error::FIELDS},
    {"8/8/8/8/8/8/4K3 w - - 0 1", fen_error::PLACEMENT},      // 7 ranks
    {"4k4/8/8/8/8/8/8/4K3 w - - 0 1", fen_error::PLACEMENT},  // 9 squares
    {"4k3/8/8/8/8/8/8/4K2 w - - 0 1", fen_error::PLACEMENT},  // 7 squares
    {"4x3/8/8/8/8/8/8/4K3 w - - 0 1", fen_error::PLACEMENT},
    {"4k3/8/8/8/8/8/8/4K3 x - - 0 1", fen_error::SIDE_TO_MOVE},
    {"4k3/8/8/8/8/8/8/4K3 w KX - 0 1", fen_error::CASTLING},
    {"4k3/8/8/8/8/8/8/4K3 w - z9 0 1", fen_error::EN_PASSANT},
    {"4k3/8/8/8/8/8/8/4K3 w - e6 0 1", fen_error::EN_PASSANT},  // no pawn
    // e3 is behind a white pawn, but White is to move: Black moved last.
    {"4k3/8/8/8/4P3/8/8/4K3 w - e3 0 1", fen_error::EN_PASSANT},
    // e6 must be empty: the pawn passed it.
    {"4k3/8/4n3/4p3/8/8/8/4K3 w - e6 0 1", fen_error::EN_PASSANT},
    {"4k3/8/8/8/8/8/8/4K3 w - - -1 1", fen_error::HALFMOVE_CLOCK},
    {"4k3/8/8/8/8/8/8/4K3 w - - x 1", fen_error::HALFMOVE_CLOCK},
    {"4k3/8/8/8/8/8/8/4K3 w - - 0 0", fen_error::FULLMOVE_NUMBER},
    {"4k3/8/8/8/8/8/8/4K3 w - - 0 -3", fen_error::FULLMOVE_NUMBER},
    {"4k3/8/8/8/8/8/8/4K3 w - - 0 99999999999", fen_error::FULLMOVE_NUMBER},
    {"4k3/8/8/8/8/8/8/8 w - - 0 1", fen_error::KINGS},
    {"4k3/8/8/8/8/8/8/3KK3 w - - 0 1", fen_error::KINGS},
    // Before the check, a pawn on rank 8 made the move generator read
    // outside the board.
    {"P3k3/8/8/8/8/8/8/4K3 w - - 0 1", fen_error::PAWN_ON_BACK_RANK},
    {"4k3/8/8/8/8/8/8/p3K3 b - - 0 1", fen_error::PAWN_ON_BACK_RANK},
    // Castling rights without rook or king in place made the engine use
    // square 8 or 255.
    {"4k3/8/8/8/8/8/8/4K2R w Q - 0 1", fen_error::CASTLING_PIECES},
    {"4k3/8/8/8/8/8/8/R2K3R w K - 0 1", fen_error::CASTLING_PIECES},
    {"r3k3/8/8/8/8/8/8/4K3 w k - 0 1", fen_error::CASTLING_PIECES},
    // Black to move, but the white king is attacked by the rook.
    {"4k3/8/8/8/8/8/8/4K2r b - - 0 1", fen_error::OPPONENT_IN_CHECK},
    {"8/8/8/8/8/8/3k4/4K3 w - - 0 1", fen_error::OPPONENT_IN_CHECK},  // kings
};

TEST(Fen, InvalidInputIsRejectedWithItsReason) {
  for (const Rejected& r : kRejected) {
    engine e;
    EXPECT_EQ(e.try_new_game(r.fen), r.error) << "\"" << r.fen << "\"";
  }
}

TEST(Fen, RejectedFenLeavesTheGameUnchanged) {
  for (const Rejected& r : kRejected) {
    engine e;
    e.new_game();
    play(e, "e2e4");
    const std::vector<std::string> moves_before = legal_uci_moves(e);
    EXPECT_NE(e.try_new_game(r.fen), fen_error::NONE) << r.fen;
    EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 1) << r.fen;
    EXPECT_EQ(e.get_color_to_move(), piece_color::BLACK) << r.fen;
    EXPECT_EQ(legal_uci_moves(e), moves_before) << r.fen;
    e.undo_move();  // the game played before is still there
    EXPECT_EQ(legal_uci_moves(e).size(), 20u) << r.fen;
  }
}

TEST(Fen, ThrowingVersionNamesTheReason) {
  engine e;
  try {
    e.new_game("P3k3/8/8/8/8/8/8/4K3 w - - 0 1");
    ADD_FAILURE() << "no exception";
  } catch (const std::invalid_argument& ex) {
    EXPECT_NE(std::string(ex.what()).find("rank 1 or 8"), std::string::npos)
        << ex.what();
  }
}

TEST(Fen, AcceptsValidSpecialCases) {
  const char* const valid[] = {
      "  rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1  ",
      "4k3/8/8/8/4P3/8/8/4K3 b - e3 0 1",      // after e2e4
      "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2",     // after d7d5
      "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 3 8",  // all castling rights
      "4k3/8/8/8/8/8/8/4K3 w - - 300 80",      // clock above 100
      "4k3/8/8/8/8/8/8/4K3 w - -",             // counters missing
      "4k3/8/8/8/8/8/8/4K3 w - - 0 1000000",   // kMaxFullmoveNumber
  };
  for (const char* fen : valid) {
    engine e;
    EXPECT_EQ(e.try_new_game(fen), fen_error::NONE) << fen;
  }
}

TEST(Fen, ErrorMessagesAreDistinct) {
  std::set<std::string> messages;
  for (int i = 0; i <= static_cast<int>(fen_error::OPPONENT_IN_CHECK); ++i) {
    messages.insert(fen_error_message(static_cast<fen_error>(i)));
  }
  EXPECT_EQ(messages.size(),
            static_cast<size_t>(fen_error::OPPONENT_IN_CHECK) + 1);
}

TEST(Fen, LoadedCheckmateIsGameOver) {
  // Fool's mate: 1. f3 e5 2. g4 Qh4#
  engine e;
  e.new_game("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
  game_history gs = e.get_current_gamestate();
  EXPECT_TRUE(gs.is_check);
  EXPECT_EQ(gs.result, BLACK_WIN);
  EXPECT_TRUE(legal_uci_moves(e).empty());
}

// FEN strings that to_fen() must give back exactly: all fields, castling
// rights in every combination, en passant squares, counters.
TEST(Fen, ToFenGivesTheLoadedFenBack) {
  const char* const fens[] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      "r3k2r/8/8/8/8/8/8/R3K2R b Kq - 3 8",
      "r3k2r/8/8/8/8/8/8/R3K2R w Qk - 0 20",
      "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2",
      "4k3/8/8/8/4P3/8/8/4K3 b - e3 0 1",
      "7k/8/8/8/8/8/8/K6Q b - - 99 250",
  };
  for (const char* fen : fens) {
    engine e;
    ASSERT_EQ(e.try_new_game(fen), fen_error::NONE) << fen;
    EXPECT_EQ(e.to_fen(), fen);
  }
}

TEST(Fen, ToFenAfterMoves) {
  engine e;
  e.new_game();
  play(e, "e2e4");
  EXPECT_EQ(e.to_fen(),
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  play(e, "c7c5");
  play(e, "g1f3");
  EXPECT_EQ(e.to_fen(),
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
  play(e, "e7e6");
  play(e, "f1e2");
  play(e, "e8e7");  // the black king moves: Black loses both rights
  EXPECT_EQ(e.to_fen(),
            "rnbq1bnr/pp1pkppp/4p3/2p5/4P3/5N2/PPPPBPPP/RNBQK2R w KQ - 2 4");
}

// Every position of random games can be saved as FEN and loaded again:
// the engine accepts its own FEN and gives the same string back.
TEST(Fen, ToFenOfRandomGamesLoadsAgain) {
  for (uint32_t seed = 1; seed <= 20; ++seed) {
    engine e;
    e.new_game();
    std::mt19937 rng(seed);
    for (int ply = 0; ply < 200; ++ply) {
      const std::string fen = e.to_fen();
      engine copy;
      ASSERT_EQ(copy.try_new_game(fen), fen_error::NONE) << fen;
      ASSERT_EQ(copy.to_fen(), fen);
      ASSERT_EQ(legal_uci_moves(copy), legal_uci_moves(e)) << fen;
      const std::vector<move> moves = e.get_all_legal_moves();
      if (moves.empty()) break;
      e.perform_move(moves[rng() % moves.size()]);
    }
  }
}

TEST(Fen, LoadedStalemateIsDraw) {
  engine e;
  e.new_game("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
  game_history gs = e.get_current_gamestate();
  EXPECT_FALSE(gs.is_check);
  EXPECT_EQ(gs.result, DRAW);
}

}  // namespace
