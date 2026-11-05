#include "ChessBoardWidget.hpp"

#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <algorithm>

namespace {

const QColor kLightSquare(240, 217, 181);
const QColor kDarkSquare(181, 136, 99);
const QColor kSelected(246, 246, 105, 170);
const QColor kLastMove(205, 210, 106, 130);
const QColor kCheck(220, 40, 40, 160);
const QColor kTarget(20, 85, 30, 110);

// How long the AI "thinks", so a human can follow its moves.
constexpr int kAiDelayMs = 300;

const char* colorName(piece_color c) {
  return c == piece_color::WHITE ? "White" : "Black";
}

}  // namespace

ChessBoardWidget::ChessBoardWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(240, 240);
  loadPieceImages();
  newGame();
}

QSize ChessBoardWidget::sizeHint() const { return QSize(640, 640); }

// ---------------------------------------------------------------- game flow

void ChessBoardWidget::newGame() {
  eng_.new_game();
  gameOverShown_ = false;
  clearSelection();
  afterMove();
}

void ChessBoardWidget::undoMove() {
  if (eng_.get_current_gamestate().num_halfmoves == 0) return;
  eng_.undo_move();
  // Against an AI, also take back its answer so the human is to move again.
  // With two AIs (or none) one move back is enough.
  bool oneAi = (ai_[0] == nullptr) != (ai_[1] == nullptr);
  if (oneAi && isAiToMove() && eng_.get_current_gamestate().num_halfmoves > 0) {
    eng_.undo_move();
  }
  gameOverShown_ = false;
  clearSelection();
  afterMove();
}

void ChessBoardWidget::setPlayer(piece_color color, const ai_info* ai) {
  int index = static_cast<int>(color);
  // Assigning a new unique_ptr deletes the previous AI automatically.
  ai_[index] = ai ? ai->create(&eng_) : nullptr;
  clearSelection();
  afterMove();
}

void ChessBoardWidget::afterMove() {
  update();  // schedules a repaint; Qt calls paintEvent() later
  emit statusChanged(statusText());

  if (isGameOver()) {
    if (!gameOverShown_) {
      gameOverShown_ = true;
      QMessageBox::information(this, "Game over", statusText());
    }
    return;
  }
  if (isAiToMove()) scheduleAiMove();
}

bool ChessBoardWidget::isGameOver() {
  return eng_.get_current_gamestate().result != RUNNING;
}

bool ChessBoardWidget::isAiToMove() {
  return ai_[static_cast<int>(eng_.get_color_to_move())] != nullptr;
}

void ChessBoardWidget::scheduleAiMove() {
  if (aiMovePending_) return;
  aiMovePending_ = true;
  // Not called directly: the timer returns to Qt's event loop first, so the
  // board is repainted before the AI moves.
  QTimer::singleShot(kAiDelayMs, this, &ChessBoardWidget::performAiMove);
}

void ChessBoardWidget::performAiMove() {
  aiMovePending_ = false;
  // The situation may have changed while the timer was running (undo, new
  // game, AI switched off).
  if (isGameOver() || !isAiToMove()) return;

  ai_template* ai = ai_[static_cast<int>(eng_.get_color_to_move())].get();
  ::move m = ai->next_move();
  if (m.type == NO_MOVE) return;
  eng_.perform_move(m);  // a promotion already contains the AI's piece
  afterMove();
}

QString ChessBoardWidget::statusText() {
  game_history gs = eng_.get_current_gamestate();
  switch (gs.result) {
    case WHITE_WIN:
      return "Checkmate. White wins.";
    case BLACK_WIN:
      return "Checkmate. Black wins.";
    case DRAW:
      return "Stalemate. The game is a draw.";
    case RUNNING:
      break;
  }
  QString text = QString("%1 to move").arg(colorName(gs.to_move));
  if (isAiToMove()) {
    const auto& ai = ai_[static_cast<int>(gs.to_move)];
    text += QString(" (%1)").arg(QString::fromStdString(ai->get_name()));
  }
  if (gs.is_check) text += ". Check!";
  return text;
}

// ------------------------------------------------------------ user clicks

void ChessBoardWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() != Qt::LeftButton || isGameOver() || isAiToMove()) {
    return;
  }
  std::optional<Square> sq = squareAt(event->position().toPoint());
  if (!sq) {
    clearSelection();
    return;
  }

  // Second click on a highlighted target square: make that move.
  auto target = std::find_if(targets_.begin(), targets_.end(), [&](::move& m) {
    return m.to_x == sq->x && m.to_y == sq->y;
  });
  if (selected_ && target != targets_.end()) {
    performHumanMove(*target);
    return;
  }

  // Otherwise: select an own piece, or clear the selection.
  piece p = eng_.get_current_board().get_piece(sq->x, sq->y);
  if (p.is_valid() && p.get_piece_color() == eng_.get_color_to_move()) {
    selectSquare(*sq);
  } else {
    clearSelection();
  }
}

void ChessBoardWidget::selectSquare(Square sq) {
  selected_ = sq;
  targets_ = eng_.get_legal_moves(sq.x, sq.y);
  update();
}

void ChessBoardWidget::clearSelection() {
  selected_.reset();
  targets_.clear();
  update();
}

void ChessBoardWidget::performHumanMove(::move m) {
  if (is_promotion(m)) {
    // A promotion is four legal moves to the same square, one per piece. Ask
    // which piece, then play the matching one of the four.
    std::optional<piece_type> type = askPromotionPiece();
    if (!type) return;  // dialog cancelled: keep the selection, no move
    m.promotion = *type;
  }
  eng_.perform_move(m);
  clearSelection();
  afterMove();
}

std::optional<piece_type> ChessBoardWidget::askPromotionPiece() {
  const QStringList names = {"Queen", "Rook", "Bishop", "Knight"};
  const piece_type types[] = {QUEEN, ROOK, BISHOP, KNIGHT};
  bool ok = false;
  QString choice = QInputDialog::getItem(
      this, "Promotion", "Promote pawn to:", names, 0, false, &ok);
  if (!ok) return std::nullopt;
  return types[names.indexOf(choice)];
}

// ---------------------------------------------------------------- geometry

int ChessBoardWidget::squareSize() const {
  return std::min(width(), height()) / 8;
}

QPoint ChessBoardWidget::boardOrigin() const {
  int side = 8 * squareSize();
  return QPoint((width() - side) / 2, (height() - side) / 2);
}

QRect ChessBoardWidget::squareRect(int x, int y) const {
  int s = squareSize();
  QPoint o = boardOrigin();
  // Rank 8 (y = 7) is the top screen row.
  return QRect(o.x() + x * s, o.y() + (7 - y) * s, s, s);
}

std::optional<ChessBoardWidget::Square> ChessBoardWidget::squareAt(
    const QPoint& pos) const {
  int s = squareSize();
  QPoint rel = pos - boardOrigin();
  if (s == 0 || rel.x() < 0 || rel.y() < 0 || rel.x() >= 8 * s ||
      rel.y() >= 8 * s) {
    return std::nullopt;  // click next to the board
  }
  return Square{rel.x() / s, 7 - rel.y() / s};
}

// ---------------------------------------------------------------- painting

void ChessBoardWidget::loadPieceImages() {
  const char letters[] = "kqbnrp";  // same order as enum piece_type
  for (int color = 0; color < 2; ++color) {
    for (int type = 0; type < 6; ++type) {
      // ":/" means Qt's resource system: the images are compiled into the
      // binary (see qt_add_resources in CMakeLists.txt).
      QString file = QString(":/images/%1_%2.png")
                         .arg(color == 0 ? "white" : "black")
                         .arg(letters[type]);
      QPixmap& img = pieceImages_[color * 6 + type];
      if (!img.load(file))
        qWarning("Cannot load piece image %s", qPrintable(file));
    }
  }
}

const QPixmap& ChessBoardWidget::imageFor(piece p) const {
  int color = static_cast<int>(p.get_piece_color());
  return pieceImages_[color * 6 + p.get_piece_type()];
}

void ChessBoardWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  board& b = eng_.get_current_board();
  game_history gs = eng_.get_current_gamestate();
  int s = squareSize();

  // 1. Squares. a1 (0, 0) is dark.
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      painter.fillRect(squareRect(x, y),
                       (x + y) % 2 == 0 ? kDarkSquare : kLightSquare);
    }
  }

  // 2. Highlights: last move, king in check, selected piece. The engine stores
  //    the last move in the previous game state.
  if (gs.prev && gs.prev->performed_move.mover.is_valid()) {
    const ::move& last = gs.prev->performed_move;
    painter.fillRect(squareRect(last.from_x, last.from_y), kLastMove);
    painter.fillRect(squareRect(last.to_x, last.to_y), kLastMove);
  }
  if (gs.is_check) {
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        piece p = b.get_piece(x, y);
        if (p.is_valid() && p.get_piece_type() == KING &&
            p.get_piece_color() == gs.to_move) {
          painter.fillRect(squareRect(x, y), kCheck);
        }
      }
    }
  }
  if (selected_)
    painter.fillRect(squareRect(selected_->x, selected_->y), kSelected);

  // 3. Pieces.
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      piece p = b.get_piece(x, y);
      if (p.is_valid()) painter.drawPixmap(squareRect(x, y), imageFor(p));
    }
  }

  // 4. Targets of the selected piece: a dot on empty squares, a ring around
  //    pieces that can be captured.
  painter.setPen(Qt::NoPen);
  painter.setBrush(kTarget);
  for (::move& m : targets_) {
    QRectF r = squareRect(m.to_x, m.to_y);
    if (b.get_piece(m.to_x, m.to_y).is_valid() || m.type == ENPASSANT) {
      QPainterPath ring;
      ring.addEllipse(r);
      ring.addEllipse(r.adjusted(s * 0.08, s * 0.08, -s * 0.08, -s * 0.08));
      painter.drawPath(ring);  // odd-even fill: only the ring is painted
    } else {
      painter.drawEllipse(r.center(), s * 0.15, s * 0.15);
    }
  }

  // 5. Coordinates: files along the bottom row, ranks along the left column.
  QFont labelFont = painter.font();
  labelFont.setPixelSize(std::max(9, s / 7));
  labelFont.setBold(true);
  painter.setFont(labelFont);
  for (int i = 0; i < 8; ++i) {
    QRect file = squareRect(i, 0).adjusted(0, 0, -3, -2);
    painter.setPen(i % 2 == 0 ? kLightSquare : kDarkSquare);
    painter.drawText(file, Qt::AlignRight | Qt::AlignBottom,
                     QString(QChar('a' + i)));
    QRect rank = squareRect(0, i).adjusted(3, 2, 0, 0);
    painter.setPen(i % 2 == 0 ? kLightSquare : kDarkSquare);
    painter.drawText(rank, Qt::AlignLeft | Qt::AlignTop,
                     QString::number(i + 1));
  }
}
