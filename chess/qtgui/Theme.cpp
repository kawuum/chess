#include "Theme.hpp"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QStyleHints>

namespace theme {

namespace {

bool dark = false;

QPalette lightPalette() {
  QPalette p;
  p.setColor(QPalette::Window, QColor("#f4f4f1"));
  p.setColor(QPalette::WindowText, QColor("#1f1f1e"));
  p.setColor(QPalette::Base, QColor("#ffffff"));
  p.setColor(QPalette::AlternateBase, QColor("#ebebe6"));
  p.setColor(QPalette::Text, QColor("#1f1f1e"));
  p.setColor(QPalette::PlaceholderText, QColor("#8a8a85"));
  p.setColor(QPalette::Button, QColor("#ffffff"));
  p.setColor(QPalette::ButtonText, QColor("#1f1f1e"));
  p.setColor(QPalette::Mid, QColor("#d3d1c7"));
  p.setColor(QPalette::Dark, QColor("#b4b2a9"));
  p.setColor(QPalette::Highlight, QColor("#1d9e75"));
  p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
  p.setColor(QPalette::ToolTipBase, QColor("#2c2c2a"));
  p.setColor(QPalette::ToolTipText, QColor("#f1efe8"));
  p.setColor(QPalette::Disabled, QPalette::Text, QColor("#b4b2a9"));
  p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#b4b2a9"));
  p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#b4b2a9"));
  return p;
}

QPalette darkPalette() {
  QPalette p;
  p.setColor(QPalette::Window, QColor("#1e1f22"));
  p.setColor(QPalette::WindowText, QColor("#e6e6e3"));
  p.setColor(QPalette::Base, QColor("#26282c"));
  p.setColor(QPalette::AlternateBase, QColor("#30333a"));
  p.setColor(QPalette::Text, QColor("#e6e6e3"));
  p.setColor(QPalette::PlaceholderText, QColor("#888780"));
  p.setColor(QPalette::Button, QColor("#2d2f33"));
  p.setColor(QPalette::ButtonText, QColor("#e6e6e3"));
  p.setColor(QPalette::Mid, QColor("#44464b"));
  p.setColor(QPalette::Dark, QColor("#5f5e5a"));
  p.setColor(QPalette::Highlight, QColor("#2fb98a"));
  p.setColor(QPalette::HighlightedText, QColor("#0b0b0b"));
  p.setColor(QPalette::ToolTipBase, QColor("#f1efe8"));
  p.setColor(QPalette::ToolTipText, QColor("#2c2c2a"));
  p.setColor(QPalette::Disabled, QPalette::Text, QColor("#5f5e5a"));
  p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#5f5e5a"));
  p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#5f5e5a"));
  return p;
}

// Rounded cards (group boxes), buttons and a slim progress bar. palette(...)
// refers to the colors above, so the sheet works for light and dark.
const char* const kStyleSheet = R"(
QGroupBox {
  border: 1px solid palette(mid);
  border-radius: 10px;
  margin-top: 16px;
  padding: 12px 10px 10px 10px;
  background: palette(base);
}
QGroupBox::title {
  subcontrol-origin: margin;
  left: 12px;
  padding: 0 4px;
  color: palette(placeholder-text);
}
QPushButton {
  border: 1px solid palette(mid);
  border-radius: 6px;
  padding: 6px 16px;
  background: palette(button);
}
QPushButton:hover { border-color: palette(dark); }
QPushButton:disabled { color: palette(placeholder-text); }
QPushButton#start {
  background: palette(highlight);
  color: palette(highlighted-text);
  border-color: palette(highlight);
}
QPushButton#start:disabled {
  background: palette(alternate-base);
  color: palette(placeholder-text);
  border-color: palette(mid);
}
QProgressBar {
  border: none;
  border-radius: 4px;
  background: palette(alternate-base);
  max-height: 8px;
  text-align: center;
  color: transparent;
}
QProgressBar::chunk { background: palette(highlight); border-radius: 4px; }
QStatusBar { color: palette(placeholder-text); }
)";

void applyColors(QApplication& app) {
  // CHESS_THEME=dark or =light overrides the system setting.
  const QString forced = qEnvironmentVariable("CHESS_THEME");
  if (forced == "dark" || forced == "light") {
    dark = forced == "dark";
  } else {
    dark = app.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
  }
  app.setPalette(dark ? darkPalette() : lightPalette());
}

}  // namespace

void apply(QApplication& app) {
  app.setStyle(QStyleFactory::create("Fusion"));
  applyColors(app);
  app.setStyleSheet(kStyleSheet);
  // The system switched between light and dark (e.g. in GNOME's settings).
  QObject::connect(app.styleHints(), &QStyleHints::colorSchemeChanged, &app,
                   [&app] { applyColors(app); });
}

bool isDark() { return dark; }

QColor accent() { return QApplication::palette().color(QPalette::Highlight); }

}  // namespace theme
