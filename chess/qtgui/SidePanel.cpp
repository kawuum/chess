#include "SidePanel.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

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
  layout->addWidget(createCard(piece_color::WHITE, white_));

  connect(board_, &ChessBoardWidget::statusChanged, this, &SidePanel::refresh);
  connect(board_, &ChessBoardWidget::movesChanged, this, &SidePanel::refresh);
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

  auto* text = new QVBoxLayout;
  text->setSpacing(0);
  text->addWidget(card.name);
  text->addWidget(card.state);
  auto* row = new QHBoxLayout(card.frame);
  row->setContentsMargins(12, 10, 12, 10);
  row->setSpacing(10);
  row->addWidget(card.marker);
  row->addLayout(text, 1);
  row->addWidget(card.lead);
  return card.frame;
}

void SidePanel::refresh() {
  refreshCard(piece_color::WHITE, white_);
  refreshCard(piece_color::BLACK, black_);
  refreshMoves();
}

void SidePanel::refreshCard(piece_color color, PlayerCard& card) {
  card.name->setText(colorName(color) + " · " + board_->playerName(color));

  const bool toMove = board_->colorToMove() == color;
  QString state;
  if (board_->isGameOver()) {
    state = board_->statusText();
  } else if (toMove && board_->isWaitingForClick()) {
    state = "click the board to go on";
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
