#include "MainWindow.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include "ChessBoardWidget.hpp"
#include "SidePanel.hpp"
#include "SimulationDialog.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), board_(new ChessBoardWidget(this)) {
  setWindowTitle("Chess");
  // The board on the left, the side panel (players, moves) on the right.
  auto* central = new QWidget(this);
  auto* layout = new QHBoxLayout(central);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(16);
  layout->addWidget(board_, 1);
  layout->addWidget(new SidePanel(board_, central));
  setCentralWidget(central);

  // Every QAction gets the window as parent, so Qt deletes it with the window.
  QMenu* game = menuBar()->addMenu("&Game");

  QAction* newGame = game->addAction("&New Game");
  newGame->setShortcut(QKeySequence::New);
  connect(newGame, &QAction::triggered, board_, &ChessBoardWidget::newGame);

  QAction* undo = game->addAction("&Undo Move");
  undo->setShortcut(QKeySequence::Undo);
  connect(undo, &QAction::triggered, board_, &ChessBoardWidget::undoMove);

  game->addSeparator();

  QAction* simulate = game->addAction("&Simulate Games...");
  connect(simulate, &QAction::triggered, this, &MainWindow::showSimulation);

  game->addSeparator();

  QAction* quit = game->addAction("&Quit");
  quit->setShortcut(QKeySequence::Quit);
  connect(quit, &QAction::triggered, qApp, &QApplication::quit);

  QMenu* players = menuBar()->addMenu("&Players");
  addPlayerMenu(players, "&White", piece_color::WHITE);
  addPlayerMenu(players, "&Black", piece_color::BLACK);

  QMenu* options = menuBar()->addMenu("&Options");
  addAiSpeedMenu(options);

  QMenu* help = menuBar()->addMenu("&Help");
  QAction* about = help->addAction("&About Chess");
  connect(about, &QAction::triggered, this,
          [this] { QMessageBox::about(this, "About Chess", aboutText()); });

  connect(board_, &ChessBoardWidget::statusChanged, statusBar(),
          [this](const QString& text) { statusBar()->showMessage(text); });
  // The board emitted its first status in its constructor, before this
  // connection existed, so show it once by hand.
  statusBar()->showMessage(board_->statusText());
}

void MainWindow::addPlayerMenu(QMenu* parent, const QString& title,
                               piece_color color) {
  QMenu* menu = parent->addMenu(title);
  // An exclusive QActionGroup works like radio buttons: checking one entry
  // unchecks the others.
  auto* group = new QActionGroup(menu);

  QAction* human = menu->addAction("Human");
  human->setCheckable(true);
  human->setChecked(true);
  group->addAction(human);
  connect(human, &QAction::triggered, board_,
          [this, color] { board_->setPlayer(color, nullptr); });

  menu->addSeparator();
  for (const ai_info& ai : available_ais()) {
    QAction* action = menu->addAction(QString::fromStdString(ai.name));
    action->setCheckable(true);
    group->addAction(action);
    // The pointer stays valid: available_ais() is a static list.
    const ai_info* info = &ai;
    connect(action, &QAction::triggered, board_,
            [this, color, info] { board_->setPlayer(color, info); });
  }
}

QString MainWindow::aboutText() {
  // The license of the piece images (BSD) asks that its notice comes with
  // the program, so the full text is shown here. It is compiled in with the
  // images (see CMakeLists.txt).
  QFile license(":/images/PIECES-LICENSE.txt");
  QString notice = license.open(QIODevice::ReadOnly | QIODevice::Text)
                       ? QString::fromUtf8(license.readAll())
                       : QString("(license text missing)");
  return "<h3>Chess</h3>"
         "<p>A chess program with a Qt GUI and several AIs.<br>"
         "github.com/kawuum/chess</p>"
         "<p>Chess pieces by Colin M.L. Burnett (Cburnett), Wikimedia "
         "Commons, used under the BSD license:</p><pre style=\"font-size: "
         "small\">" +
         notice.toHtmlEscaped() + "</pre>";
}

void MainWindow::showSimulation() {
  // Created on first use and then kept: closing only hides it, so the last
  // results are still there when it is opened again. Not modal: the board
  // stays usable while games are simulated.
  if (simulation_ == nullptr) simulation_ = new SimulationDialog(this);
  simulation_->show();
  simulation_->raise();
  simulation_->activateWindow();
}

void MainWindow::addAiSpeedMenu(QMenu* parent) {
  QMenu* menu = parent->addMenu("&AI vs AI speed");
  auto* group = new QActionGroup(menu);  // radio buttons, see addPlayerMenu

  struct Choice {
    const char* text;
    int delayMs;
  };
  const Choice choices[] = {
      {"Wait for click", ChessBoardWidget::kWaitForClick},
      {"0.3 s per move", 300},
      {"1 s per move", 1000},
      {"2 s per move", 2000},
  };
  for (const Choice& choice : choices) {
    QAction* action = menu->addAction(choice.text);
    action->setCheckable(true);
    group->addAction(action);
    int delay = choice.delayMs;
    connect(action, &QAction::triggered, board_,
            [this, delay] { board_->setAiVsAiDelay(delay); });
  }
  // The widget starts with "wait for click".
  group->actions().first()->setChecked(true);
}
