#include "SidePanel.hpp"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "../ai/ai_registry.hpp"
#include "ChessBoardWidget.hpp"

namespace {

QString colorName(piece_color c) {
  return c == piece_color::WHITE ? "White" : "Black";
}

}  // namespace

SidePanel::SidePanel(ChessBoardWidget* board, QWidget* parent)
    : QWidget(parent), board_(board) {
  setMinimumWidth(240);
  setMaximumWidth(300);

  // The moves: one row per full move, columns number / White / Black.
  moves_ = new QTableWidget(0, 3, this);
  moves_->setObjectName("moves");
  moves_->horizontalHeader()->hide();
  moves_->verticalHeader()->hide();
  moves_->setShowGrid(false);
  moves_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  moves_->setSelectionMode(QAbstractItemView::SingleSelection);
  moves_->setFocusPolicy(Qt::NoFocus);
  moves_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  moves_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  moves_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  moves_->setColumnWidth(0, 40);
  moves_->setStyleSheet(
      "QTableWidget { border: 1px solid palette(mid); border-radius: 10px;"
      " background: palette(base); padding: 6px; }"
      "QTableWidget::item { padding: 2px 6px; border-radius: 4px; }"
      "QTableWidget::item:selected { background: palette(highlight);"
      " color: palette(highlighted-text); }");
  QFont mono("monospace");
  mono.setStyleHint(QFont::Monospace);
  moves_->setFont(mono);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(10);
  layout->addWidget(createCard(piece_color::BLACK, black_));
  layout->addWidget(moves_, 1);
  layout->addWidget(createResultCard());
  layout->addWidget(createCard(piece_color::WHITE, white_));

  connect(board_, &ChessBoardWidget::statusChanged, this, &SidePanel::refresh);
  connect(board_, &ChessBoardWidget::movesChanged, this, &SidePanel::refresh);
  connect(board_, &ChessBoardWidget::playersChanged, this, [this] {
    refreshPlayer(piece_color::WHITE, white_);
    refreshPlayer(piece_color::BLACK, black_);
  });
  refresh();
}

QWidget* SidePanel::createCard(piece_color color, PlayerCard& card) {
  const bool white = color == piece_color::WHITE;
  const QString prefix = white ? "white" : "black";

  card.frame = new QFrame(this);
  card.frame->setObjectName(prefix + "Card");

  // A round marker in the player's color, with a king on it.
  card.marker =
      new QLabel(white ? QString(QChar(0x2654)) : QChar(0x265A), card.frame);
  card.marker->setFixedSize(36, 36);
  card.marker->setAlignment(Qt::AlignCenter);
  card.marker->setStyleSheet(
      white ? "background: #f4f4f1; color: #2c2c2a; border: 1px solid "
              "#b4b2a9; border-radius: 18px; font-size: 20px;"
            : "background: #2c2c2a; color: #f4f4f1; border: 1px solid "
              "#888780; border-radius: 18px; font-size: 20px;");

  card.name = new QLabel(card.frame);
  card.name->setObjectName(prefix + "Name");
  QFont bold = card.name->font();
  bold.setBold(true);
  card.name->setFont(bold);
  card.state = new QLabel(card.frame);
  card.state->setObjectName(prefix + "State");
  card.state->setStyleSheet("color: palette(placeholder-text);");
  card.lead = new QLabel(card.frame);
  card.lead->setObjectName(prefix + "Lead");
  card.lead->setStyleSheet(
      "background: palette(alternate-base); border-radius: 6px;"
      " padding: 2px 8px;");

  card.check = new QLabel("Check", card.frame);
  card.check->setObjectName(prefix + "Check");
  card.check->setStyleSheet(
      "background: #c62828; color: white; font-weight: bold;"
      " border-radius: 6px; padding: 2px 8px;");
  card.check->hide();

  card.next = new QPushButton("Next move", card.frame);
  card.next->setObjectName(prefix + "Next");
  card.next->setToolTip(
      "Plays the next move of this AI. Space or a click on the board does\n"
      "the same. Options > AI vs AI speed lets the AIs play by themselves.");
  card.next->hide();
  connect(card.next, &QPushButton::clicked, board_,
          [this] { board_->playNextAiMove(); });

  auto* text = new QVBoxLayout;
  text->setSpacing(0);
  text->addWidget(card.name);
  text->addWidget(card.state);
  // Who plays this color: Human or one of the AIs, with the depth for AIs
  // that search.
  card.player = new QComboBox(card.frame);
  card.player->setObjectName(prefix + "Player");
  card.player->addItem("Human");
  for (const ai_info& ai : available_ais()) {
    card.player->addItem(QString::fromStdString(ai.name));
  }
  card.depthLabel = new QLabel("Depth", card.frame);
  card.depth = new QSpinBox(card.frame);
  card.depth->setObjectName(prefix + "Depth");
  card.depth->setToolTip(
      "How many half moves the AI looks ahead. Deeper plays better but\n"
      "takes longer; the window waits while the AI thinks.");
  card.depthLabel->setToolTip(card.depth->toolTip());
  connect(card.player, &QComboBox::currentIndexChanged, this,
          [this, color, &card] { choosePlayer(color, card); });
  connect(card.depth, &QSpinBox::valueChanged, this,
          [this, color, &card] { choosePlayer(color, card); });

  auto* top = new QHBoxLayout;
  top->setSpacing(10);
  top->addWidget(card.marker);
  top->addLayout(text, 1);
  top->addWidget(card.check);
  top->addWidget(card.next);
  top->addWidget(card.lead);
  auto* choice = new QHBoxLayout;
  choice->setSpacing(6);
  choice->addWidget(card.player, 1);
  choice->addWidget(card.depthLabel);
  choice->addWidget(card.depth);
  auto* rows = new QVBoxLayout(card.frame);
  rows->setContentsMargins(12, 10, 12, 10);
  rows->setSpacing(8);
  rows->addLayout(top);
  rows->addLayout(choice);
  refreshPlayer(color, card);
  return card.frame;
}

QWidget* SidePanel::createResultCard() {
  result_ = new QFrame(this);
  result_->setObjectName("resultCard");
  result_->setStyleSheet(
      "#resultCard { background: palette(base); border-radius: 10px;"
      " border: 2px solid palette(highlight); }");
  resultTitle_ = new QLabel(result_);
  resultTitle_->setObjectName("resultTitle");
  QFont big = resultTitle_->font();
  big.setBold(true);
  big.setPointSizeF(big.pointSizeF() * 1.4);
  resultTitle_->setFont(big);
  resultReason_ = new QLabel(result_);
  resultReason_->setObjectName("resultReason");
  auto* newGame = new QPushButton("New Game", result_);
  newGame->setObjectName("resultNewGame");
  connect(newGame, &QPushButton::clicked, board_, &ChessBoardWidget::newGame);
  auto* layout = new QVBoxLayout(result_);
  layout->setContentsMargins(12, 10, 12, 10);
  layout->setSpacing(4);
  layout->addWidget(resultTitle_);
  layout->addWidget(resultReason_);
  layout->addSpacing(6);
  layout->addWidget(newGame);
  result_->hide();
  return result_;
}

void SidePanel::refreshResult() {
  const game_result result = board_->result();
  result_->setVisible(result != RUNNING);
  if (result == RUNNING) return;
  if (result == DRAW) {
    resultTitle_->setText("Draw");
    switch (board_->drawReason()) {
      case STALEMATE:
        resultReason_->setText("by stalemate");
        break;
      case THREEFOLD_REPETITION:
        resultReason_->setText("threefold repetition");
        break;
      case FIFTY_MOVE_RULE:
        resultReason_->setText("50-move rule");
        break;
      case INSUFFICIENT_MATERIAL:
        resultReason_->setText("insufficient material");
        break;
      case NO_DRAW:
        resultReason_->clear();
        break;
    }
  } else {
    resultTitle_->setText(result == WHITE_WIN ? "White wins" : "Black wins");
    resultReason_->setText("by checkmate");
  }
}

void SidePanel::refresh() {
  refreshCard(piece_color::WHITE, white_);
  refreshCard(piece_color::BLACK, black_);
  refreshMoves();
  refreshResult();
}

void SidePanel::refreshPlayer(piece_color color, PlayerCard& card) {
  // Setting the controls must not count as a choice of the user.
  const QSignalBlocker blockPlayer(card.player);
  const QSignalBlocker blockDepth(card.depth);
  const ai_info* info = board_->playerInfo(color);
  int index = 0;  // Human
  const std::vector<ai_info>& ais = available_ais();
  for (size_t i = 0; i < ais.size(); ++i) {
    if (&ais[i] == info) index = static_cast<int>(i) + 1;
  }
  card.player->setCurrentIndex(index);
  const bool hasDepth = info != nullptr && info->create_with_depth != nullptr;
  card.depthLabel->setVisible(hasDepth);
  card.depth->setVisible(hasDepth);
  if (hasDepth) {
    card.depth->setRange(1, info->max_depth_interactive);
    card.depth->setValue(board_->playerDepth(color));
  }
}

void SidePanel::choosePlayer(piece_color color, PlayerCard& card) {
  const int index = card.player->currentIndex();
  const ai_info* info = index > 0 ? &available_ais()[index - 1] : nullptr;
  // A new AI starts with its default depth; the same AI keeps the chosen one.
  const int depth = info == board_->playerInfo(color) ? card.depth->value() : 0;
  board_->setPlayer(color, info, depth);  // refreshPlayer() follows
}

void SidePanel::refreshCard(piece_color color, PlayerCard& card) {
  card.name->setText(colorName(color));

  const bool toMove = board_->colorToMove() == color;
  QString state;
  if (board_->isGameOver()) {
    state = board_->statusText();
  } else if (toMove && board_->isWaitingForClick()) {
    state = "waiting for you";
  } else if (toMove && board_->isAiToMove()) {
    state = "thinking...";
  } else if (toMove) {
    state = "your move";
  }
  card.state->setText(state);
  card.state->setVisible(!state.isEmpty());  // the name moves to the middle

  // The material lead is shown on the card of the side that is ahead.
  const int balance = board_->materialBalance();
  const int lead = color == piece_color::WHITE ? balance : -balance;
  card.lead->setText(QString("+%1").arg(lead));
  card.lead->setVisible(lead > 0);
  card.check->setVisible(toMove && board_->isCheck() && !board_->isGameOver());
  card.next->setVisible(toMove && board_->isWaitingForClick());

  // The side to move gets a frame in the accent color.
  card.frame->setStyleSheet(
      QString("#%1 { background: palette(base); border-radius: 10px;"
              " border: %2; }")
          .arg(card.frame->objectName(), toMove && !board_->isGameOver()
                                             ? "2px solid palette(highlight)"
                                             : "1px solid palette(mid)"));
}

void SidePanel::refreshMoves() {
  const QStringList& moves = board_->moveList();
  // Half move number of the first entry, counted from 0 = White's first.
  const int first = board_->firstHalfmove();
  // A game from a position with Black to move starts in the Black column.
  const int cells = moves.size() + first % 2;
  moves_->clearContents();  // after an undo, cells may have to go
  moves_->setRowCount((cells + 1) / 2);
  for (int i = 0; i < moves.size(); ++i) {
    const int halfmove = first + i;
    const int row = (halfmove - first + first % 2) / 2;
    const int column = 1 + halfmove % 2;  // 1 = White, 2 = Black
    moves_->setItem(row, 0,
                    new QTableWidgetItem(QString("%1.").arg(halfmove / 2 + 1)));
    moves_->item(row, 0)->setForeground(palette().placeholderText());
    moves_->item(row, 0)->setFlags(Qt::ItemIsEnabled);
    moves_->setItem(row, column, new QTableWidgetItem(moves[i]));
  }
  if (moves.isEmpty()) {
    moves_->clearSelection();
  } else {
    // The last move is selected (highlighted) and scrolled into view.
    const int last = first + moves.size() - 1;
    const int row = (last - first + first % 2) / 2;
    moves_->setCurrentCell(row, 1 + last % 2);
    moves_->scrollToItem(moves_->item(row, 1 + last % 2));
  }
}
