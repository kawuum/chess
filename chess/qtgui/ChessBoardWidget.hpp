#ifndef CHESS_CHESSBOARDWIDGET_HPP
#define CHESS_CHESSBOARDWIDGET_HPP

#include <QPixmap>
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

 signals:
  // Short text for the status bar, e.g. "White to move".
  void statusChanged(const QString& text);

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

  void loadPieceImages();
  const QPixmap& imageFor(piece p) const;

  void selectSquare(Square sq);
  void clearSelection();
  void performHumanMove(::move m);
  std::optional<piece_type> askPromotionPiece();

  // Called after every move: repaint, update status, show game over or let
  // the AI move next.
  void afterMove();
  bool isGameOver();
  bool isAiToMove();
  bool isAiVsAi();
  bool aiVsAiWaitsForClick();
  void scheduleAiMove();
  void onAiTimer();
  void performAiMove();

  engine eng_;
  // AI per color, indexed by (int)piece_color. nullptr means human.
  std::array<std::unique_ptr<ai_template>, 2> ai_;
  // Index: color * 6 + piece_type.
  std::array<QPixmap, 12> pieceImages_;

  std::optional<Square> selected_;
  std::vector<::move> targets_;   // legal moves of the selected piece
  bool aiMovePending_ = false;    // a timer for the next AI move is running
  bool waitingForClick_ = false;  // the next AI move comes with a click
  int aiVsAiDelay_ = kWaitForClick;
  bool gameOverShown_ = false;
  PromotionChooser promotionChooser_;  // empty: use the dialog
};

#endif  // CHESS_CHESSBOARDWIDGET_HPP
