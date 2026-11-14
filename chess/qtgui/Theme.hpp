#pragma once

#include <QColor>
#include <QPalette>

class QApplication;

// The look of the whole program: Qt's "Fusion" style with our own colors, in
// a light and a dark variant that follows the system setting, plus a small
// style sheet (rounded cards and buttons, a slim progress bar).
namespace theme {

// Applies the theme and keeps it in sync when the system switches between
// light and dark. The environment variable CHESS_THEME=dark or =light
// overrides the system setting.
void apply(QApplication& app);

// True if the dark variant is active.
bool isDark();

// The accent color (teal), e.g. for the bar of a progress or a highlight.
QColor accent();

// The colors of the light or the dark variant (for tests: apply() picks one).
QPalette palette(bool dark);

// The colors of the board, the same in both variants.
struct BoardColors {
  QColor lightSquare;
  QColor darkSquare;
  QColor lastMove;  // translucent tint of the squares of the last move
  QColor selected;  // translucent tint of the square of the selected piece
  // Frame around a tinted square: the tint alone is too faint (below 3:1),
  // the dark frame stands out on both tinted square colors.
  QColor mark;
  QColor target;  // translucent dot or ring on the targets of a piece
  QColor check;   // center of the red glow under a king in check
  QColor edge;    // thin line around the board
};
const BoardColors& boardColors();

// WCAG 2.1: the relative luminance of a color (0 = black, 1 = white) and
// the contrast of two colors, from 1:1 (same) to 21:1 (black on white).
// Level AA asks 4.5:1 for text and 3:1 for graphics that carry meaning.
double luminance(const QColor& color);
double contrast(const QColor& a, const QColor& b);
// The color seen when the translucent color over is drawn on under.
QColor blend(const QColor& over, const QColor& under);

}  // namespace theme
