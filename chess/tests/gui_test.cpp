// GUI tests: drive MainWindow and ChessBoardWidget with simulated mouse
// clicks. They run without a screen on Qt's "offscreen" platform.
#include <gtest/gtest.h>

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTest>
#include <algorithm>
#include <ostream>

#include "../ai/ai_registry.hpp"
#include "../qtgui/ChessBoardWidget.hpp"
#include "../qtgui/MainWindow.hpp"

// Lets GoogleTest print a QString as text when a test fails, instead of its
// raw bytes. GoogleTest finds this function by name (it must be in the same
// namespace as QString, the global one).
void PrintTo(const QString& s, std::ostream* os) {
  *os << '"' << s.toStdString() << '"';
}

namespace {

// Center of a board square in widget coordinates. Same geometry as
// ChessBoardWidget: square board, centered, white at the bottom.
QPoint center(QWidget& w, int x, int y) {
  int s = std::min(w.width(), w.height()) / 8;
  int ox = (w.width() - 8 * s) / 2;
  int oy = (w.height() - 8 * s) / 2;
  return QPoint(ox + x * s + s / 2, oy + (7 - y) * s + s / 2);
}

QAction* find_action(QMenu* menu, const QString& text) {
  for (QAction* a : menu->actions()) {
    if (a->text() == text) return a;
  }
  return nullptr;
}

class Gui : public testing::Test {
 protected:
  void SetUp() override {
    window.resize(640, 700);
    window.show();
    board = window.findChild<ChessBoardWidget*>();
    ASSERT_NE(board, nullptr);
  }

  void click(int x, int y) {
    QTest::mouseClick(board, Qt::LeftButton, Qt::NoModifier,
                      center(*board, x, y));
  }

  QString status() { return window.statusBar()->currentMessage(); }

  // The submenu Players > White/Black.
  QMenu* player_menu(const QString& color) {
    for (QAction* a : window.menuBar()->actions()) {
      if (a->text() == "&Players") {
        QAction* sub = find_action(a->menu(), color);
        return sub ? sub->menu() : nullptr;
      }
    }
    return nullptr;
  }

  // An entry of Options > AI vs AI speed.
  QAction* ai_speed_action(const QString& text) {
    for (QAction* a : window.menuBar()->actions()) {
      if (a->text() == "&Options") {
        QAction* sub = find_action(a->menu(), "&AI vs AI speed");
        return sub ? find_action(sub->menu(), text) : nullptr;
      }
    }
    return nullptr;
  }

  // Both colors played by the Random AI, chosen in the menu like a user does.
  void random_ai_for_both() {
    find_action(player_menu("&White"), "Random AI")->trigger();
    find_action(player_menu("&Black"), "Random AI")->trigger();
  }

  MainWindow window;
  ChessBoardWidget* board = nullptr;
};

TEST_F(Gui, OpensAsASingleWindow) {
  // The old board opened every square as its own window.
  int visible = 0;
  for (QWidget* w : QApplication::topLevelWidgets()) {
    if (w->isVisible()) ++visible;
  }
  EXPECT_EQ(visible, 1);
}

TEST_F(Gui, StartsWithWhiteToMove) { EXPECT_EQ(status(), "White to move"); }

TEST_F(Gui, PieceImagesAreLoaded) {
  // The pixel in the middle of e2 belongs to the pawn image, not to the
  // square. If the embedded images were missing, it would show the square.
  QImage img = board->grab().toImage();
  QColor pawn = img.pixelColor(center(*board, 4, 1));
  QColor empty_light = img.pixelColor(center(*board, 4, 3));  // e4
  EXPECT_NE(pawn, empty_light);
}

TEST_F(Gui, ClickTwiceMovesAPiece) {
  click(4, 1);  // e2
  click(4, 3);  // e4
  EXPECT_EQ(status(), "Black to move");
}

TEST_F(Gui, IllegalTargetDoesNotMove) {
  click(4, 1);  // e2
  click(4, 4);  // e5: too far for a pawn
  EXPECT_EQ(status(), "White to move");
}

TEST_F(Gui, UndoTakesTheMoveBack) {
  click(4, 1);
  click(4, 3);
  board->undoMove();
  EXPECT_EQ(status(), "White to move");
}

TEST_F(Gui, PlayersMenuListsHumanAndAllAis) {
  QMenu* black = player_menu("&Black");
  ASSERT_NE(black, nullptr);
  EXPECT_NE(find_action(black, "Human"), nullptr);
  for (const ai_info& ai : available_ais()) {
    EXPECT_NE(find_action(black, QString::fromStdString(ai.name)), nullptr);
  }
  EXPECT_TRUE(find_action(black, "Human")->isChecked());
}

TEST_F(Gui, AiChosenInMenuAnswersAMove) {
  QMenu* black = player_menu("&Black");
  ASSERT_NE(black, nullptr);
  find_action(black, "Random AI")->trigger();
  click(4, 1);
  click(4, 3);
  EXPECT_EQ(status(), "Black to move (Random AI)");
  // The AI moves after a short delay (QTimer), so wait for it.
  EXPECT_TRUE(
      QTest::qWaitFor([&] { return status() == "White to move"; }, 2000));
}

TEST_F(Gui, AiSpeedStartsWithWaitForClick) {
  QAction* wait = ai_speed_action("Wait for click");
  ASSERT_NE(wait, nullptr);
  EXPECT_TRUE(wait->isChecked());
}

TEST_F(Gui, AiVsAiWaitsForAClick) {
  random_ai_for_both();
  // Switching White to an AI first started the timer of a game against a
  // human. It runs out during this wait, but must not make a move.
  QTest::qWait(600);
  EXPECT_EQ(status(),
            "White to move (Random AI) - click the board for the next move");
  click(4, 4);  // any square
  EXPECT_EQ(status(),
            "Black to move (Random AI) - click the board for the next move");
  QTest::qWait(600);  // still nothing happens without a click
  EXPECT_EQ(status(),
            "Black to move (Random AI) - click the board for the next move");
}

TEST_F(Gui, AiVsAiWithDelayPlaysByItself) {
  random_ai_for_both();
  // Chosen while the game waits for a click: it goes on by itself.
  ai_speed_action("0.3 s per move")->trigger();
  EXPECT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("Black to move"); }, 2000));
  EXPECT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("White to move"); }, 2000));
  EXPECT_FALSE(status().contains("click"));
}

TEST_F(Gui, NaiveAiTakesAHangingQueen) {
  // After 1.e4 d5 the human plays 2.Qg4??; the naive AI (Black) must answer
  // with Bc8xg4.
  find_action(player_menu("&Black"), "Naive AI")->trigger();
  board->loadPosition(
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
  click(3, 0);  // d1
  click(6, 3);  // g4
  ASSERT_TRUE(QTest::qWaitFor(
      [&] { return status().startsWith("White to move"); }, 2000));
  piece p = board->pieceAt(6, 3);  // g4
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), BISHOP);
  EXPECT_EQ(p.get_piece_color(), piece_color::BLACK);
}

// White pawn on a7, one step before promotion.
const char* const kPromotionFen = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1";

TEST_F(Gui, PromotionPlaysTheChosenPiece) {
  // Instead of the dialog, a function chooses the knight.
  int asked = 0;
  board->setPromotionChooser([&asked]() -> std::optional<piece_type> {
    ++asked;
    return KNIGHT;
  });
  board->loadPosition(kPromotionFen);
  click(0, 6);  // a7
  click(0, 7);  // a8
  EXPECT_EQ(asked, 1);
  piece p = board->pieceAt(0, 7);
  ASSERT_TRUE(p.is_valid());
  EXPECT_EQ(p.get_piece_type(), KNIGHT);
  EXPECT_EQ(p.get_piece_color(), piece_color::WHITE);
  EXPECT_EQ(status(), "Black to move");
}

TEST_F(Gui, CancelledPromotionKeepsThePosition) {
  board->setPromotionChooser([] { return std::optional<piece_type>(); });
  board->loadPosition(kPromotionFen);
  click(0, 6);
  click(0, 7);
  EXPECT_EQ(board->pieceAt(0, 6).get_piece_type(), PAWN);  // still on a7
  EXPECT_FALSE(board->pieceAt(0, 7).is_valid());           // a8 empty
  EXPECT_EQ(status(), "White to move");
}

}  // namespace

int main(int argc, char** argv) {
  // No screen needed. A real display can still be chosen by setting
  // QT_QPA_PLATFORM before starting the tests.
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }
  testing::InitGoogleTest(&argc, argv);
  QApplication app(argc, argv);
  return RUN_ALL_TESTS();
}
