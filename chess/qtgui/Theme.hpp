#pragma once

#include <QColor>

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

}  // namespace theme
