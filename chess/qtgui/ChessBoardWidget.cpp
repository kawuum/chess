#include "ChessBoardWidget.hpp"

#include <QImageReader>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QVariantAnimation>
#include <algorithm>
#include <random>
#include <utility>

#include "../engine/evaluation.hpp"
#include "../engine/notation.hpp"
#include "Theme.hpp"

namespace {

constexpr qreal kCornerRadius = 6;  // rounded corners of the board
constexpr int kSlideMs = 150;       // how long a piece slides to its square

// How long the AI "thinks" in a game against a human, so the human can
// follow its moves. Games between two AIs use setAiVsAiDelay() instead.
constexpr int kAiDelayMs = 300;

const char* colorName(piece_color c) {
  return c == piece_color::WHITE ? "White" : "Black";
}

const char* drawText(draw_reason reason) {
  switch (reason) {
    case STALEMATE:
      return "Stalemate. The game is a draw.";
    case THREEFOLD_REPETITION:
      return "Threefold repetition. The game is a draw.";
    case FIFTY_MOVE_RULE:
      return "50 moves without capture or pawn move. The game is a draw.";
    case INSUFFICIENT_MATERIAL:
      return "Not enough material to checkmate. The game is a draw.";
    case NO_DRAW:
      break;
  }
  return "The game is a draw.";
}

}  // namespace

ChessBoardWidget::ChessBoardWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(240, 240);
  newGame();  // the piece images are rendered at the first paint
}

QSize ChessBoardWidget::sizeHint() const { return QSize(640, 640); }

// ---------------------------------------------------------------- game flow

void ChessBoardWidget::newGame() {
  eng_.new_game();
  startedNewGame();
}

void ChessBoardWidget::loadPosition(const std::string& fen) {
  eng_.new_game(fen);
  startedNewGame();
}

fen_error ChessBoardWidget::tryLoadPosition(const std::string& fen) {
  const fen_error error = eng_.try_new_game(fen);
  if (error == fen_error::NONE) startedNewGame();
  return error;
}

QString ChessBoardWidget::fen() const {
  return QString::fromStdString(eng_.to_fen());
}

void ChessBoardWidget::startedNewGame() {
  // A position from a FEN may be from the middle of a game: the move list
  // then starts with that move number.
  firstHalfmove_ = eng_.get_current_gamestate().num_halfmoves;
  moveList_.clear();
  stopAnimation();
  clearSelection();
  emit movesChanged();
  afterMove();
}

void ChessBoardWidget::playMove(const ::move& m) {
  // The notation needs the position before the move ("Nbd2", "exd5"), the
  // animation the one after it.
  moveList_.push_back(QString::fromStdString(to_san(eng_, m)));
  eng_.perform_move(m);
  startAnimation(m);
  emit movesChanged();
  afterMove();
}

QString ChessBoardWidget::playerName(piece_color color) const {
  const auto& ai = ai_[static_cast<int>(color)];
  return ai ? QString::fromStdString(ai->get_name()) : QString("Human");
}

piece_color ChessBoardWidget::colorToMove() { return eng_.get_color_to_move(); }

int ChessBoardWidget::materialBalance() {
  const board& b = eng_.get_current_board();
  return evaluation::material(b, piece_color::WHITE) -
         evaluation::material(b, piece_color::BLACK);
}

piece ChessBoardWidget::pieceAt(int x, int y) {
  return eng_.get_current_board().get_piece(x, y);
}

void ChessBoardWidget::setPromotionChooser(PromotionChooser chooser) {
  promotionChooser_ = std::move(chooser);
}

void ChessBoardWidget::setAiVsAiDelay(int ms) {
  aiVsAiDelay_ = ms;
  // Waiting for a click, but clicks are no longer wanted: go on by itself.
  if (waitingForClick_ && !aiVsAiWaitsForClick()) {
    waitingForClick_ = false;
    scheduleAiMove();
  }
  emit statusChanged(statusText());
}

void ChessBoardWidget::undoMove() {
  // Every move of this game is in the move list, so an empty list means
  // there is nothing to take back (also after loading a position).
  if (moveList_.isEmpty()) return;
  eng_.undo_move();
  moveList_.pop_back();
  // Against an AI, also take back its answer so the human is to move again.
  // With two AIs (or none) one move back is enough.
  bool oneAi = (ai_[0] == nullptr) != (ai_[1] == nullptr);
  if (oneAi && isAiToMove() && !moveList_.isEmpty()) {
    eng_.undo_move();
    moveList_.pop_back();
  }
  stopAnimation();
  clearSelection();
  emit movesChanged();
  afterMove();
}

void ChessBoardWidget::setPlayer(piece_color color, const ai_info* ai,
                                 int depth) {
  int index = static_cast<int>(color);
  info_[index] = ai;
  depth_[index] =
      ai && ai->create_with_depth ? (depth > 0 ? depth : ai->default_depth) : 0;
  // Assigning a new unique_ptr deletes the previous AI automatically.
  // A new random seed every time: a game in the GUI should not be the same
  // as the last one.
  ai_[index] = ai ? create_ai(*ai, &eng_, std::random_device{}(), depth_[index])
                  : nullptr;
  emit playersChanged();
  clearSelection();
  afterMove();
}

void ChessBoardWidget::afterMove() {
  update();  // schedules a repaint; Qt calls paintEvent() later

  // Decide first whether an AI moves next (and whether it waits for a click),
  // so that the status text below already says so.
  waitingForClick_ = false;
  if (!isGameOver() && isAiToMove()) scheduleAiMove();
  // A finished game is shown in the side panel (no modal box: it would
  // stop everything until clicked).
  emit statusChanged(statusText());
}

bool ChessBoardWidget::isGameOver() {
  return eng_.get_current_gamestate().result != RUNNING;
}

bool ChessBoardWidget::isAiToMove() {
  return ai_[static_cast<int>(eng_.get_color_to_move())] != nullptr;
}

bool ChessBoardWidget::isAiVsAi() {
  return ai_[0] != nullptr && ai_[1] != nullptr;
}

bool ChessBoardWidget::aiVsAiWaitsForClick() {
  return isAiVsAi() && aiVsAiDelay_ == kWaitForClick;
}

void ChessBoardWidget::scheduleAiMove() {
  if (aiMovePending_ || waitingForClick_) return;  // already on its way
  if (aiVsAiWaitsForClick()) {
    waitingForClick_ = true;  // mousePressEvent() makes the move
    return;
  }
  aiMovePending_ = true;
  int delay = isAiVsAi() ? aiVsAiDelay_ : kAiDelayMs;
  // Not called directly: the timer returns to Qt's event loop first, so the
  // board is repainted before the AI moves.
  QTimer::singleShot(delay, this, &ChessBoardWidget::onAiTimer);
}

void ChessBoardWidget::onAiTimer() {
  aiMovePending_ = false;
  // While the timer was running, the game may have become AI against AI with
  // "wait for click" (e.g. the second color was just switched to an AI).
  if (aiVsAiWaitsForClick() && !isGameOver()) {
    waitingForClick_ = true;
    emit statusChanged(statusText());
    return;
  }
  performAiMove();
}

void ChessBoardWidget::performAiMove() {
  aiMovePending_ = false;
  waitingForClick_ = false;
  // The situation may have changed while the timer was running (undo, new
  // game, AI switched off).
  if (isGameOver() || !isAiToMove()) return;

  ai_template* ai = ai_[static_cast<int>(eng_.get_color_to_move())].get();
  ::move m = ai->next_move();
  if (m.type == NO_MOVE) return;
  playMove(m);  // a promotion already contains the AI's piece
}

QString ChessBoardWidget::statusText() {
  game_history gs = eng_.get_current_gamestate();
  switch (gs.result) {
    case WHITE_WIN:
      return "Checkmate. White wins.";
    case BLACK_WIN:
      return "Checkmate. Black wins.";
    case DRAW:
      return drawText(gs.draw);
    case RUNNING:
      break;
  }
  // Short: who plays which color is on the player cards.
  QString text = QString("%1 to move").arg(colorName(gs.to_move));
  if (gs.is_check) text += ". Check!";
  if (waitingForClick_) text += " - click the board for the next move";
  return text;
}

// ------------------------------------------------------------ user clicks

void ChessBoardWidget::mousePressEvent(QMouseEvent* event) {
  // AI against AI, waiting for a click: anywhere on the widget plays the
  // next AI move.
  if (event->button() == Qt::LeftButton && waitingForClick_) {
    performAiMove();
    return;
  }
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
  clearSelection();
  playMove(m);
}

std::optional<piece_type> ChessBoardWidget::askPromotionPiece() {
  if (promotionChooser_) return promotionChooser_();
  const QStringList names = {"Queen", "Rook", "Bishop", "Knight"};
  const piece_type types[] = {QUEEN, ROOK, BISHOP, KNIGHT};
  bool ok = false;
  QString choice = QInputDialog::getItem(
      this, "Promotion", "Promote pawn to:", names, 0, false, &ok);
  if (!ok) return std::nullopt;
  return types[names.indexOf(choice)];
}

// ---------------------------------------------------------------- geometry

QSize ChessBoardWidget::labelSpace() const {
  const QFontMetrics metrics(font());
  return QSize(metrics.horizontalAdvance("8") + 8, metrics.height() + 4);
}

int ChessBoardWidget::squareSize() const {
  const QSize labels = labelSpace();
  return std::max(0, std::min(width() - labels.width(),
                              height() - labels.height())) /
         8;
}

QPoint ChessBoardWidget::boardOrigin() const {
  // The board with the ranks on its left and the files below it, centered.
  const QSize labels = labelSpace();
  const int side = 8 * squareSize();
  return QPoint((width() - side - labels.width()) / 2 + labels.width(),
                (height() - side - labels.height()) / 2);
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

void ChessBoardWidget::renderPieceImages(int size) {
  // The pieces are SVG (vector) images: they are rendered once for the
  // current square size, so they stay sharp at every size. On a screen with
  // scaling (e.g. 2x on a Retina display) a square of 80 points has 160
  // pixels, so the images are rendered with that many pixels.
  const qreal ratio = devicePixelRatioF();
  const int pixels = qRound(size * ratio);
  const char letters[] = "kqbnrp";  // same order as enum piece_type
  for (int color = 0; color < 2; ++color) {
    for (int type = 0; type < 6; ++type) {
      // ":/" means Qt's resource system: the images are compiled into the
      // binary (see qt_add_resources in CMakeLists.txt). Qt reads SVG with
      // its image format plugin (package qt6-svg-plugins).
      QString file = QString(":/images/%1_%2.svg")
                         .arg(color == 0 ? "white" : "black")
                         .arg(letters[type]);
      QImageReader reader(file);
      reader.setScaledSize(QSize(pixels, pixels));
      QImage image = reader.read();
      if (image.isNull()) {
        qWarning("Cannot load piece image %s: %s", qPrintable(file),
                 qPrintable(reader.errorString()));
      }
      QPixmap& pixmap = pieceImages_[color * 6 + type];
      pixmap = QPixmap::fromImage(image);
      pixmap.setDevicePixelRatio(ratio);
    }
  }
  renderedSize_ = size;
  renderedRatio_ = ratio;
}

const QPixmap& ChessBoardWidget::imageFor(piece p) const {
  int color = static_cast<int>(p.get_piece_color());
  return pieceImages_[color * 6 + p.get_piece_type()];
}

// ---------------------------------------------------------------- animation

void ChessBoardWidget::startAnimation(const ::move& m) {
  // The engine has made the move already; only the picture lags behind for a
  // moment. The piece now on the target square slides there (for a
  // promotion that is the new piece); when castling, the rook slides too.
  slides_.clear();
  const board& b = eng_.get_current_board();
  slides_.push_back({b.get_piece(m.to_x, m.to_y), Square{m.from_x, m.from_y},
                     Square{m.to_x, m.to_y}});
  if (m.type == CASTLING) {
    const bool kingSide = m.to_x > m.from_x;
    const int rookFrom = kingSide ? 7 : 0;
    const int rookTo = kingSide ? 5 : 3;
    slides_.push_back({b.get_piece(rookTo, m.to_y), Square{rookFrom, m.to_y},
                       Square{rookTo, m.to_y}});
  }
  if (animation_ == nullptr) {
    animation_ = new QVariantAnimation(this);
    animation_->setStartValue(0.0);
    animation_->setEndValue(1.0);
    animation_->setDuration(kSlideMs);
    animation_->setEasingCurve(QEasingCurve::OutCubic);
    // Every new value repaints; at the end the slides are done.
    connect(animation_, &QVariantAnimation::valueChanged, this,
            [this] { update(); });
    connect(animation_, &QVariantAnimation::finished, this, [this] {
      slides_.clear();
      update();
    });
  }
  animation_->stop();
  animation_->start();
}

void ChessBoardWidget::stopAnimation() {
  if (animation_ != nullptr) animation_->stop();
  slides_.clear();
  update();
}

// ---------------------------------------------------------------- painting

void ChessBoardWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  board& b = eng_.get_current_board();
  game_history gs = eng_.get_current_gamestate();
  const int s = squareSize();
  if (s != renderedSize_ || devicePixelRatioF() != renderedRatio_) {
    renderPieceImages(s);  // first paint, or the window was resized
  }
  const QRectF boardRect(boardOrigin(), QSizeF(8 * s, 8 * s));

  const theme::BoardColors& colors = theme::boardColors();

  // The board has rounded corners: everything on it is clipped to them.
  QPainterPath rounded;
  rounded.addRoundedRect(boardRect, kCornerRadius, kCornerRadius);
  painter.setClipPath(rounded);

  // 1. Squares. a1 (0, 0) is dark.
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      painter.fillRect(squareRect(x, y), (x + y) % 2 == 0 ? colors.darkSquare
                                                          : colors.lightSquare);
    }
  }

  // 2. Highlights: last move, selected piece, each a tint with a frame.
  //    The engine stores the last move in the previous game state.
  const qreal frameWidth = std::max(2.0, s / 22.0);
  auto mark = [&](int x, int y, const QColor& tint) {
    const QRectF r = squareRect(x, y);
    painter.fillRect(r, tint);
    painter.setPen(QPen(colors.mark, frameWidth));
    painter.setBrush(Qt::NoBrush);
    const qreal inset = frameWidth / 2;
    painter.drawRect(r.adjusted(inset, inset, -inset, -inset));
  };
  if (gs.prev && gs.prev->performed_move.mover.is_valid()) {
    const ::move& last = gs.prev->performed_move;
    mark(last.from_x, last.from_y, colors.lastMove);
    mark(last.to_x, last.to_y, colors.lastMove);
  }
  if (selected_) mark(selected_->x, selected_->y, colors.selected);

  // 3. King in check: a red glow under the king, strongest in the middle.
  if (gs.is_check) {
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        piece p = b.get_piece(x, y);
        if (p.is_valid() && p.get_piece_type() == KING &&
            p.get_piece_color() == gs.to_move) {
          QRectF r = squareRect(x, y);
          QRadialGradient glow(r.center(), s * 0.6);
          glow.setColorAt(0.0, colors.check);
          glow.setColorAt(1.0, Qt::transparent);
          painter.fillRect(r, glow);
        }
      }
    }
  }

  // 5. Pieces. A piece that is still sliding is drawn in step 7 instead.
  auto sliding_to = [this](int x, int y) {
    for (const Slide& slide : slides_) {
      if (slide.to.x == x && slide.to.y == y) return true;
    }
    return false;
  };
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      piece p = b.get_piece(x, y);
      if (p.is_valid() && !sliding_to(x, y)) {
        painter.drawPixmap(squareRect(x, y), imageFor(p));
      }
    }
  }

  // 6. Targets of the selected piece: a dot on empty squares, a ring around
  //    pieces that can be captured.
  painter.setPen(Qt::NoPen);
  painter.setBrush(colors.target);
  for (::move& m : targets_) {
    QRectF r = squareRect(m.to_x, m.to_y);
    if (b.get_piece(m.to_x, m.to_y).is_valid() || m.type == ENPASSANT) {
      QPainterPath ring;
      ring.addEllipse(r);
      ring.addEllipse(r.adjusted(s * 0.08, s * 0.08, -s * 0.08, -s * 0.08));
      painter.drawPath(ring);  // odd-even fill: only the ring is painted
    } else {
      painter.drawEllipse(r.center(), s * 0.16, s * 0.16);
    }
  }

  // 7. Sliding pieces, on top of everything, between start and target.
  if (!slides_.empty() && animation_ != nullptr) {
    const qreal t = animation_->currentValue().toReal();
    for (const Slide& slide : slides_) {
      QPointF from = squareRect(slide.from.x, slide.from.y).topLeft();
      QPointF to = squareRect(slide.to.x, slide.to.y).topLeft();
      QPointF at = from + (to - from) * t;
      const QPixmap& image = imageFor(slide.moving);
      painter.drawPixmap(QRectF(at, QSizeF(s, s)), image, QRectF(image.rect()));
    }
  }

  // 8. A thin edge around the board.
  painter.setClipping(false);
  painter.setPen(QPen(colors.edge, 1.0));
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(boardRect.adjusted(0.5, 0.5, -0.5, -0.5),
                          kCornerRadius, kCornerRadius);

  // 9. Coordinates next to the board, on the window background: ranks on
  //    the left, files below. Inside the squares small letters could not
  //    reach a contrast of 4.5:1 on both square colors.
  painter.setFont(font());
  painter.setPen(palette().color(QPalette::PlaceholderText));
  const QSize labels = labelSpace();
  for (int i = 0; i < 8; ++i) {
    const QRect file = squareRect(i, 0);
    painter.drawText(QRect(file.left(), file.bottom() + 1, s, labels.height()),
                     Qt::AlignCenter, QString(QChar('a' + i)));
    const QRect rank = squareRect(0, i);
    painter.drawText(
        QRect(rank.left() - labels.width(), rank.top(), labels.width() - 6, s),
        Qt::AlignRight | Qt::AlignVCenter, QString::number(i + 1));
  }
}
