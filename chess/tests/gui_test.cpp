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

#include "../ai/ai_registry.hpp"
#include "../qtgui/ChessBoardWidget.hpp"
#include "../qtgui/MainWindow.hpp"

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
