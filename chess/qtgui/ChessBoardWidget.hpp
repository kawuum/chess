#ifndef CHESS_CHESSBOARDWIDGET_HPP
#define CHESS_CHESSBOARDWIDGET_HPP

#include <QPixmap>
#include <QStringList>
#include <QWidget>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../ai/ai_registry.hpp"
#include "../ai/template/ai_template.hpp"
#include "../engine/engine.hpp"

// One widget that draws the whole board and lets the user play by clicking.
//
// Board coordinates follow the engine: x is the file (0 = a), y is the rank
// (0 = rank 1). White is drawn at the bottom, so screen rows are flipped.
class QVariantAnimation;

class ChessBoardWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChessBoardWidget(QWidget* parent = nullptr);

  void newGame();
  // Starts a game from the given position (FEN). Throws std::invalid_argument
  // for an invalid FEN; the current game is unchanged then.
  void loadPosition(const std::string& fen);
  // The piece on square (x, y), an invalid piece if the square is empty.
  piece pieceAt(int x, int y);
  // Takes back the last move. Against an AI it takes back the AI's answer
  // too, so the human is to move again.
  void undoMove();
  // Chooses who plays the given color: an AI from available_ais(), or a
  // human if ai is nullptr.
  void setPlayer(piece_color color, const ai_info* ai);
  // Whose turn it is, check, or how the game ended.
  QString statusText();

  // How the promotion piece of a human move is chosen. Normally a dialog asks
  // the user; an empty result cancels the move. Tests set a function instead,
  // because a dialog cannot be clicked without a screen.
  using PromotionChooser = std::function<std::optional<piece_type>()>;
  void setPromotionChooser(PromotionChooser chooser);

  // Pace of games between two AIs, so a human can follow them: every AI move
  // waits for a click on the board (kWaitForClick, the default), or comes
  // after a delay in milliseconds. Against a human the AI always moves after
  // a short fixed delay.
  static constexpr int kWaitForClick = -1;
  void setAiVsAiDelay(int ms);

  // For the side panel: the moves of this game in algebraic notation ("e4",
  // "Nf3", ...), and how many half moves were played before the first of
  // them (more than 0 if the game started from a loaded position).
  const QStringList& moveList() const { return moveList_; }
  int firstHalfmove() const { return firstHalfmove_; }
  // "Human" or the name of the AI that plays the color.
  QString playerName(piece_color color) const;
  piece_color colorToMove();
  bool isGameOver();
  // An AI plays the side to move; it may be waiting for a click.
  bool isAiToMove();
  bool isWaitingForClick() const { return waitingForClick_; }
  // A timer for the next AI move is running (for tests: wait until it has
  // run out instead of waiting a fixed time).
  bool isAiMovePending() const { return aiMovePending_; }
  // Material of White minus material of Black, in pawns (e.g. +3).
  int materialBalance();

 signals:
  // Short text for the status bar, e.g. "White to move".
  void statusChanged(const QString& text);
  // A move was played or taken back, or a new game started.
  void movesChanged();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  QSize sizeHint() const override;

 private:
  struct Square {
    int x;
    int y;
  };

  // Geometry: the board is a square, centered in the widget.
  int squareSize() const;
  QPoint boardOrigin() const;
  QRect squareRect(int x, int y) const;
  std::optional<Square> squareAt(const QPoint& pos) const;

  // Renders the 12 SVG piece images for squares of the given size.
  void renderPieceImages(int size);
  const QPixmap& imageFor(piece p) const;

  void selectSquare(Square sq);
  void clearSelection();
  // Every move of the game goes through here: notation, engine, animation.
  void playMove(const ::move& m);
  // After newGame() and loadPosition(): empty move list, no animation.
  void startedNewGame();
  void performHumanMove(::move m);
  std::optional<piece_type> askPromotionPiece();

  // Called after every move: repaint, update status, show game over or let
  // the AI move next.
  void afterMove();
  bool isAiVsAi();
  bool aiVsAiWaitsForClick();
  void scheduleAiMove();
  void onAiTimer();
  void performAiMove();

  // A move is shown as a short slide of the piece. The engine has made the
  // move already; only the drawing lags behind.
  struct Slide {
    piece moving;  // the piece as it stands on the target square
    Square from;
    Square to;
  };
  void startAnimation(const ::move& m);
  void stopAnimation();

  engine eng_;
  // AI per color, indexed by (int)piece_color. nullptr means human.
  std::array<std::unique_ptr<ai_template>, 2> ai_;
  // Index: color * 6 + piece_type. Rendered for renderedSize_ (square size
  // in points) and renderedRatio_ (pixels per point of the screen).
  std::array<QPixmap, 12> pieceImages_;
  int renderedSize_ = 0;
  qreal renderedRatio_ = 0;

  std::optional<Square> selected_;
  std::vector<::move> targets_;   // legal moves of the selected piece
  bool aiMovePending_ = false;    // a timer for the next AI move is running
  bool waitingForClick_ = false;  // the next AI move comes with a click
  int aiVsAiDelay_ = kWaitForClick;
  bool gameOverShown_ = false;
  PromotionChooser promotionChooser_;  // empty: use the dialog

  QStringList moveList_;
  int firstHalfmove_ = 0;
  std::vector<Slide> slides_;               // pieces sliding right now
  QVariantAnimation* animation_ = nullptr;  // 0 to 1 while sliding
};

#endif  // CHESS_CHESSBOARDWIDGET_HPP
