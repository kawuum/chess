#include "MainWindow.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
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

  QAction* copyFen = game->addAction("&Copy Position (FEN)");
  copyFen->setShortcut(QKeySequence("Ctrl+Shift+C"));
  connect(copyFen, &QAction::triggered, this, &MainWindow::copyFen);

  QAction* loadFen = game->addAction("&Load Position (FEN)...");
  loadFen->setShortcut(QKeySequence("Ctrl+L"));
  connect(loadFen, &QAction::triggered, this, &MainWindow::loadFen);

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
  // A short note (e.g. "copied") disappears after a few seconds; then the
  // board's status comes back.
  connect(statusBar(), &QStatusBar::messageChanged, this,
          [this](const QString& text) {
            if (text.isEmpty()) statusBar()->showMessage(board_->statusText());
          });

  // The real input dialog; tests replace it with setFenAsker().
  fenAsker_ = [this](const QString& suggestion,
                     const QString& problem) -> std::optional<QString> {
    QString label = "Position in FEN, e.g. copied from a simulation:";
    if (!problem.isEmpty()) label = problem + "\n\n" + label;
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, "Load Position", label, QLineEdit::Normal, suggestion, &ok);
    if (!ok) return std::nullopt;
    return text;
  };
}

void MainWindow::copyFen() {
  QGuiApplication::clipboard()->setText(board_->fen());
  statusBar()->showMessage("Position copied to the clipboard (FEN)", 3000);
}

void MainWindow::loadFen() {
  // Suggest the clipboard if it holds a valid FEN (e.g. just copied from
  // another program), otherwise the current position to edit.
  const QString clipboard = QGuiApplication::clipboard()->text().trimmed();
  engine probe;
  QString suggestion =
      probe.try_new_game(clipboard.toStdString()) == fen_error::NONE
          ? clipboard
          : board_->fen();
  QString problem;
  while (true) {
    const std::optional<QString> text = fenAsker_(suggestion, problem);
    if (!text) return;  // cancelled: the game goes on
    const fen_error error =
        board_->tryLoadPosition(text->trimmed().toStdString());
    if (error == fen_error::NONE) return;
    // Ask again with the reason and the rejected text, to correct it.
    problem = fen_error_message(error);
    suggestion = *text;
  }
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
  human->setData(0);  // 0 = Human, i + 1 = available_ais()[i]
  group->addAction(human);
  connect(human, &QAction::triggered, board_,
          [this, color] { board_->setPlayer(color, nullptr); });

  menu->addSeparator();
  const std::vector<ai_info>& ais = available_ais();
  for (size_t i = 0; i < ais.size(); ++i) {
    const ai_info& ai = ais[i];
    QAction* action = menu->addAction(QString::fromStdString(ai.name));
    action->setCheckable(true);
    action->setData(static_cast<int>(i) + 1);
    group->addAction(action);
    // The pointer stays valid: available_ais() is a static list.
    const ai_info* info = &ai;
    connect(action, &QAction::triggered, board_,
            [this, color, info] { board_->setPlayer(color, info); });
  }

  // The player cards choose too: keep the check mark on the current player.
  connect(board_, &ChessBoardWidget::playersChanged, group,
          [this, color, group] {
            const ai_info* info = board_->playerInfo(color);
            const int index =
                info ? static_cast<int>(info - available_ais().data()) + 1 : 0;
            for (QAction* action : group->actions()) {
              if (action->data().toInt() == index) action->setChecked(true);
            }
          });
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
