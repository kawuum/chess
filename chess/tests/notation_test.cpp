// Tests for standard algebraic notation (engine/notation.hpp).
#include "../engine/notation.hpp"

#include <string>

#include "test_helpers.hpp"

namespace {

// The SAN of a move given in UCI notation, in the position of fen.
std::string san(const std::string& fen, const std::string& uci) {
  engine e;
  e.new_game(fen);
  game_history gh = e.get_current_gamestate();
  for (const move& m : e.get_all_legal_moves(&gh)) {
    if (to_uci(m) == uci) return to_san(e, m);
  }
  return "illegal: " + uci;
}

const char* const kStart =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

TEST(Notation, PawnAndPieceMoves) {
  EXPECT_EQ(san(kStart, "e2e4"), "e4");
  EXPECT_EQ(san(kStart, "g1f3"), "Nf3");
}

TEST(Notation, Captures) {
  // After 1.e4 d5: exd5.
  EXPECT_EQ(san("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
                "e4d5"),
            "exd5");
  // After 1.e4 e5 2.Nf3 Nc6 3.Bb5 a6: Bxc6.
  EXPECT_EQ(
      san("r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4",
          "b5c6"),
      "Bxc6");
}

TEST(Notation, EnPassant) {
  EXPECT_EQ(san("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6"), "exd6");
}

TEST(Notation, Castling) {
  const char* fen = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
  EXPECT_EQ(san(fen, "e1g1"), "O-O");
  EXPECT_EQ(san(fen, "e1c1"), "O-O-O");
}

TEST(Notation, TwoPiecesCanReachTheSquare) {
  // Knights b1 and f1 can both go to d2: the file tells them apart.
  EXPECT_EQ(san("4k3/8/8/8/8/8/8/1N2KN2 w - - 0 1", "b1d2"), "Nbd2");
  // Rooks a1 and a5 on the same file: the rank tells them apart.
  EXPECT_EQ(san("4k3/8/8/R7/8/8/8/R3K3 w - - 0 1", "a1a3"), "R1a3");
  // Queens a1, a3 and c1 all reach b2: a1 shares its file with a3 and its
  // rank with c1, so it needs both.
  EXPECT_EQ(san("4k3/8/8/8/8/Q7/8/Q1Q1K3 w - - 0 1", "a1b2"), "Qa1b2");
}

TEST(Notation, OnlyOnePieceNoDisambiguation) {
  // The black pawn is only there so the position is no draw (king and
  // knight against king would be insufficient material).
  EXPECT_EQ(san("4k3/7p/8/8/8/8/8/1N2K3 w - - 0 1", "b1d2"), "Nd2");
}

TEST(Notation, Promotion) {
  const char* fen = "3r1k2/4P3/8/8/8/8/8/4K3 w - - 0 1";
  EXPECT_EQ(san(fen, "e7e8q"), "e8=Q+");   // the queen checks the king f8
  EXPECT_EQ(san(fen, "e7d8n"), "exd8=N");  // the knight does not
}

TEST(Notation, CheckAndMate) {
  EXPECT_EQ(san("4k3/8/8/8/8/8/8/R3K3 w - - 0 1", "a1a8"), "Ra8+");
  EXPECT_EQ(san("7k/8/6K1/8/8/8/8/5Q2 w - - 0 1", "f1f8"), "Qf8#");
}

TEST(Notation, LeavesThePositionUnchanged) {
  engine e;
  e.new_game(kStart);
  game_history gh = e.get_current_gamestate();
  move m = e.get_all_legal_moves(&gh).front();
  to_san(e, m);
  EXPECT_TRUE(e.get_current_board() == gh.curr_board);
  EXPECT_EQ(e.get_current_gamestate().num_halfmoves, 0);
}

}  // namespace
