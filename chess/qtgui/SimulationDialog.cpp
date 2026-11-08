#include "SimulationDialog.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QElapsedTimer>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QThread>
#include <QVBoxLayout>
#include <exception>
#include <random>
#include <utility>

namespace {

// Seeds the user can type, and the range random seeds are drawn from.
constexpr int kMaxSeed = 999999999;

}  // namespace

// The results as one bar: wins of the first AI, draws, unfinished games and
// wins of the second AI, each as wide as its share of the games played.
class ResultBar : public QWidget {
 public:
  explicit ResultBar(QWidget* parent) : QWidget(parent) { setFixedHeight(14); }
  void setStats(const match_stats& stats) {
    stats_ = stats;
    update();
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF all = rect();
    QPainterPath rounded;
    rounded.addRoundedRect(all, height() / 2.0, height() / 2.0);
    painter.setClipPath(rounded);
    painter.fillRect(all, palette().alternateBase());  // no games yet
    if (stats_.games == 0) return;

    const struct {
      int count;
      QColor color;
    } parts[] = {
        {stats_.first_wins, palette().color(QPalette::Highlight)},
        {stats_.draws, QColor("#b4b2a9")},
        {stats_.unfinished, palette().color(QPalette::Mid)},
        {stats_.second_wins, QColor("#d85a30")},
    };
    qreal x = 0;
    for (const auto& part : parts) {
      const qreal w = all.width() * part.count / stats_.games;
      painter.fillRect(QRectF(x, 0, w, all.height()), part.color);
      x += w;
    }
  }

 private:
  match_stats stats_;
};

SimulationDialog::SimulationDialog(QWidget* parent, std::vector<ai_info> ais)
    : QDialog(parent), ais_(std::move(ais)) {
  setWindowTitle("Simulate games");
  setMinimumWidth(640);

  // 1. The two AIs, side by side: "First AI  vs  Second AI".
  auto* vs = new QLabel("vs", this);
  vs->setAlignment(Qt::AlignCenter);
  vs->setStyleSheet("color: palette(placeholder-text); font-size: 18px;");
  auto* choosers = new QHBoxLayout;
  choosers->addWidget(createChooser("White", "first", first_), 1);
  choosers->addWidget(vs);
  choosers->addWidget(createChooser("Black", "second", second_), 1);

  // 2. Number of games and the buttons.
  games_ = new QSpinBox(this);
  games_->setRange(1, 100000);
  games_->setValue(100);
  games_->setObjectName("games");  // object names let tests find widgets
  startButton_ = new QPushButton("Start", this);
  stopButton_ = new QPushButton("Stop", this);
  auto* closeButton = new QPushButton("Close", this);
  startButton_->setObjectName("start");  // the theme colors it (accent)
  stopButton_->setObjectName("stop");
  connect(startButton_, &QPushButton::clicked, this, &SimulationDialog::start);
  connect(stopButton_, &QPushButton::clicked, this, &SimulationDialog::stop);
  connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
  auto* controls = new QHBoxLayout;
  controls->addWidget(new QLabel("Games:", this));
  controls->addWidget(games_);
  // Off: each AI keeps its color. On: they swap after every game, which
  // shares White's advantage of the first move fairly between them.
  swapColors_ = new QCheckBox("Swap colors every game", this);
  swapColors_->setObjectName("swapColors");
  swapColors_->setToolTip(
      "Off: the AI under White always plays White.\n"
      "On: the AIs swap colors after every game, which shares White's\n"
      "advantage of the first move fairly.");
  controls->addSpacing(12);
  controls->addWidget(swapColors_);
  controls->addStretch();
  controls->addWidget(startButton_);
  controls->addWidget(stopButton_);
  controls->addWidget(closeButton);

  // 3. Progress: a slim bar and "37 / 100 games".
  progress_ = new QProgressBar(this);
  progressText_ = new QLabel(this);
  progressText_->setStyleSheet("color: palette(placeholder-text);");
  auto* progressRow = new QHBoxLayout;
  progressRow->addWidget(progress_, 1);
  progressRow->addWidget(progressText_);

  // 4. The results, as a card: the bar, four numbers, the draw reasons and
  //    the full text (selectable, e.g. to copy it).
  bar_ = new ResultBar(this);
  auto* tiles = new QHBoxLayout;
  tiles->setSpacing(10);
  tiles->addWidget(
      createTile(firstWins_, "firstWins", "Games this AI won by checkmate."));
  tiles->addWidget(
      createTile(draws_, "draws",
                 "Games that ended in a draw. The labels below say why."));
  tiles->addWidget(
      createTile(secondWins_, "secondWins", "Games this AI won by checkmate."));
  tiles->addWidget(
      createTile(speed_, "speed", "Games played per second so far."));
  auto* chipRow = new QHBoxLayout;
  chipRow->setSpacing(6);
  // What each label means, shown when the mouse rests on it. Same order as
  // in showVisuals().
  const QString explanations[] = {
      "Threefold repetition: the same position came up for the third time\n"
      "(same pieces on the same squares, same side to move, same castling\n"
      "and en passant rights). The game is a draw.",
      "50-move rule: 50 moves of each side (100 half moves) without a\n"
      "capture and without a pawn move. The game is a draw.",
      "Insufficient material: neither side can checkmate any more, e.g.\n"
      "king against king, or king and one bishop or knight against king.\n"
      "The game is a draw.",
      "Stalemate: the side to move has no legal move but is not in check.\n"
      "The game is a draw.",
      QString("Unfinished: games still running after %1 half moves. They\n"
              "are stopped and count neither as a win nor as a draw.")
          .arg(kMaxPlies),
      "Average length of a game in half moves. A half move is the move of\n"
      "one side: \"1. e4 e5\" are two half moves.",
  };
  for (size_t i = 0; i < chips_.size(); ++i) {
    QLabel*& chip = chips_[i];
    chip = new QLabel(this);
    chip->setObjectName(QString("chip%1").arg(i));
    chip->setToolTip(explanations[i]);
    chip->setStyleSheet(
        "background: palette(alternate-base); border-radius: 6px;"
        " padding: 2px 8px; color: palette(placeholder-text);");
    chipRow->addWidget(chip);
  }
  chipRow->addStretch();
  result_ = new QLabel(
      QString("Each AI plays its color, or both take turns with \"Swap "
              "colors every game\". A game still running after %1 half "
              "moves counts as unfinished.")
          .arg(kMaxPlies),
      this);
  result_->setWordWrap(true);
  result_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  result_->setStyleSheet("color: palette(placeholder-text);");

  auto* card = new QFrame(this);
  card->setObjectName("results");
  card->setStyleSheet(
      "#results { background: palette(base); border: 1px solid palette(mid);"
      " border-radius: 10px; }");
  auto* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(14, 14, 14, 14);
  cardLayout->setSpacing(10);
  cardLayout->addWidget(bar_);
  cardLayout->addLayout(tiles);
  cardLayout->addLayout(chipRow);
  cardLayout->addWidget(result_);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);
  layout->addLayout(choosers);
  layout->addLayout(controls);
  layout->addLayout(progressRow);
  layout->addWidget(card);

  showVisuals(match_stats{});
  setRunning(false);
}

QWidget* SimulationDialog::createTile(Tile& tile, const QString& name,
                                      const QString& explanation) {
  auto* frame = new QFrame(this);
  frame->setToolTip(explanation);
  frame->setObjectName(name + "Tile");
  frame->setStyleSheet(QString("#%1 { background: palette(alternate-base);"
                               " border-radius: 8px; }")
                           .arg(frame->objectName()));
  tile.caption = new QLabel(frame);
  tile.caption->setStyleSheet("color: palette(placeholder-text);");
  tile.value = new QLabel(frame);
  tile.value->setObjectName(name + "Value");
  QFont big = tile.value->font();
  big.setPointSizeF(big.pointSizeF() * 1.7);
  big.setBold(true);
  tile.value->setFont(big);
  auto* layout = new QVBoxLayout(frame);
  layout->setContentsMargins(10, 8, 10, 8);
  layout->setSpacing(0);
  layout->addWidget(tile.caption);
  layout->addWidget(tile.value);
  return frame;
}

QGroupBox* SimulationDialog::createChooser(const QString& title,
                                           const QString& name,
                                           AiChooser& chooser) {
  auto* box = new QGroupBox(title, this);

  chooser.ai = new QComboBox(box);
  for (const ai_info& ai : ais_) {
    chooser.ai->addItem(QString::fromStdString(ai.name));
  }
  chooser.ai->setObjectName(name + "Ai");

  chooser.randomSeed = new QCheckBox("Random seed", box);
  chooser.randomSeed->setChecked(true);
  chooser.randomSeed->setObjectName(name + "RandomSeed");
  chooser.seed = new QSpinBox(box);
  chooser.seed->setRange(0, kMaxSeed);
  chooser.seed->setValue(1);
  chooser.seed->setEnabled(false);  // "Random seed" is checked
  chooser.seed->setObjectName(name + "Seed");
  chooser.seed->setToolTip("The same seed plays the same games again.");
  // The checkbox greys out the field: a random seed is drawn at the start.
  connect(chooser.randomSeed, &QCheckBox::toggled, chooser.seed,
          [&chooser](bool random) { chooser.seed->setEnabled(!random); });

  chooser.seedRow = new QWidget(box);
  chooser.seedRow->setObjectName(name + "SeedRow");
  auto* seedLayout = new QHBoxLayout(chooser.seedRow);
  seedLayout->setContentsMargins(0, 0, 0, 0);
  seedLayout->addWidget(new QLabel("Seed:", chooser.seedRow));
  seedLayout->addWidget(chooser.seed, 1);
  seedLayout->addWidget(chooser.randomSeed);

  connect(chooser.ai, &QComboBox::currentIndexChanged, this,
          [this, &chooser] { updateSeedRow(chooser); });
  updateSeedRow(chooser);

  auto* layout = new QVBoxLayout(box);
  layout->addWidget(chooser.ai);
  layout->addWidget(chooser.seedRow);
  layout->addStretch();
  return box;
}

const ai_info& SimulationDialog::selectedAi(const AiChooser& chooser) const {
  return ais_[chooser.ai->currentIndex()];
}

void SimulationDialog::updateSeedRow(AiChooser& chooser) {
  chooser.seedRow->setVisible(selectedAi(chooser).uses_random);
}

uint32_t SimulationDialog::seedFor(AiChooser& chooser) {
  if (chooser.randomSeed->isChecked()) {
    std::uniform_int_distribution<int> draw(0, kMaxSeed);
    std::random_device device;
    chooser.seed->setValue(draw(device));
  }
  return static_cast<uint32_t>(chooser.seed->value());
}

SimulationDialog::~SimulationDialog() {
  // The thread uses this dialog (cancel_ and the queued calls), so it must
  // end before the dialog is gone.
  if (thread_) {
    cancel_ = true;
    thread_->wait();
  }
}

void SimulationDialog::start() {
  if (running_) return;
  if (thread_) thread_->wait();  // the last simulation has ended already

  // Pointers into ais_, which lives as long as the dialog.
  const ai_info* first = &selectedAi(first_);
  const ai_info* second = &selectedAi(second_);
  match_settings settings;
  settings.games = games_->value();
  settings.max_plies = kMaxPlies;
  settings.first_seed = seedFor(first_);
  settings.second_seed = seedFor(second_);

  settings.swap_colors = swapColors_->isChecked();

  // The names in the results. With fixed colors they carry the color; when
  // the colors swap, the same AI twice is numbered instead.
  firstName_ = first_.ai->currentText();
  secondName_ = second_.ai->currentText();
  if (!settings.swap_colors) {
    firstName_ += " (White)";
    secondName_ += " (Black)";
  } else if (firstName_ == secondName_) {
    firstName_ += " (1)";
    secondName_ += " (2)";
  }
  // Name the seeds of the AIs that use them, so the games can be repeated.
  QStringList seeds;
  if (first->uses_random) {
    seeds << QString("%1 %2").arg(firstName_).arg(settings.first_seed);
  }
  if (second->uses_random) {
    seeds << QString("%1 %2").arg(secondName_).arg(settings.second_seed);
  }
  seedsText_ = seeds.isEmpty() ? QString() : "Seeds: " + seeds.join(", ");
  plannedGames_ = settings.games;
  progress_->setRange(0, settings.games);
  progress_->setValue(0);
  result_->setText("Running...");
  progressText_->setText(QString("0 / %1 games").arg(plannedGames_));
  clock_.start();
  lastStats_ = match_stats{};
  showVisuals(lastStats_);
  cancel_ = false;
  setRunning(true);

  // Runs in the new thread. It only reads the AI list (which never changes)
  // and plays on engines of its own; everything for the widgets is queued
  // to the GUI thread.
  // An exception must not leave this function: one that nobody catches
  // ends the whole program. A bug in an AI (e.g. an illegal move) is shown
  // in the dialog instead.
  auto simulation = [this, first, second, settings] {
    try {
      match_stats stats = play_match(
          *first, *second, settings, [this](const match_stats& so_far) {
            QMetaObject::invokeMethod(
                this, [this, so_far] { showProgress(so_far); },
                Qt::QueuedConnection);
            return !cancel_;  // false stops the match
          });
      QMetaObject::invokeMethod(
          this, [this, stats] { showEnd(stats); }, Qt::QueuedConnection);
    } catch (const std::exception& error) {
      const QString message = QString::fromUtf8(error.what());
      QMetaObject::invokeMethod(
          this, [this, message] { showError(message); }, Qt::QueuedConnection);
    } catch (...) {
      QMetaObject::invokeMethod(
          this, [this] { showError("unknown error"); }, Qt::QueuedConnection);
    }
  };
  thread_.reset(QThread::create(simulation));
  thread_->start();
}

void SimulationDialog::stop() { cancel_ = true; }

void SimulationDialog::closeEvent(QCloseEvent* event) {
  stop();  // no point in playing on for a hidden dialog
  QDialog::closeEvent(event);
}

void SimulationDialog::showProgress(const match_stats& stats) {
  lastStats_ = stats;
  progress_->setValue(stats.games);
  progressText_->setText(
      QString("%1 / %2 games").arg(stats.games).arg(plannedGames_));
  showVisuals(stats);
  result_->setText(describeStats(stats));
}

void SimulationDialog::showVisuals(const match_stats& stats) {
  bar_->setStats(stats);

  // Before the first simulation there are no names yet.
  const QString first = firstName_.isEmpty() ? "White" : firstName_;
  const QString second = secondName_.isEmpty() ? "Black" : secondName_;
  firstWins_.caption->setText(first + " wins");
  firstWins_.value->setText(QString::number(stats.first_wins));
  draws_.caption->setText("Draws");
  draws_.value->setText(QString::number(stats.draws));
  secondWins_.caption->setText(second + " wins");
  secondWins_.value->setText(QString::number(stats.second_wins));
  speed_.caption->setText("Games / s");
  const double seconds = clock_.isValid() ? clock_.elapsed() / 1000.0 : 0;
  speed_.value->setText(seconds > 0 && stats.games > 0
                            ? QString::number(qRound(stats.games / seconds))
                            : QString("-"));

  chips_[0]->setText(
      QString("repetition %1").arg(stats.draws_by[THREEFOLD_REPETITION]));
  chips_[1]->setText(
      QString("50 moves %1").arg(stats.draws_by[FIFTY_MOVE_RULE]));
  chips_[2]->setText(
      QString("material %1").arg(stats.draws_by[INSUFFICIENT_MATERIAL]));
  chips_[3]->setText(QString("stalemate %1").arg(stats.draws_by[STALEMATE]));
  chips_[4]->setText(QString("unfinished %1").arg(stats.unfinished));
  chips_[5]->setText(
      stats.games > 0
          ? QString("\u00d8 %1 half moves").arg(stats.total_plies / stats.games)
          : QString("\u00d8 - half moves"));
}

void SimulationDialog::showEnd(const match_stats& stats) {
  showProgress(stats);
  if (stats.games < plannedGames_) {
    result_->setText(QString("Stopped after %1 of %2 games.\n\n")
                         .arg(stats.games)
                         .arg(plannedGames_) +
                     describeStats(stats));
  }
  setRunning(false);
  emit simulationEnded();
}

void SimulationDialog::showError(const QString& message) {
  // The progress calls queued before the error have run already (queued
  // calls keep their order), so lastStats_ holds the games played.
  result_->setText(QString("Stopped by an error after %1 games: %2\n\n")
                       .arg(lastStats_.games)
                       .arg(message) +
                   describeStats(lastStats_));
  setRunning(false);
  emit simulationEnded();
}

void SimulationDialog::setRunning(bool running) {
  running_ = running;
  startButton_->setEnabled(!running);
  stopButton_->setEnabled(running);
  games_->setEnabled(!running);
  swapColors_->setEnabled(!running);
  for (AiChooser* chooser : {&first_, &second_}) {
    chooser->ai->setEnabled(!running);
    chooser->randomSeed->setEnabled(!running);
    chooser->seed->setEnabled(!running && !chooser->randomSeed->isChecked());
  }
}

QString SimulationDialog::describeStats(const match_stats& stats) const {
  QString text = QString::fromStdString(
      describe(stats, firstName_.toStdString(), secondName_.toStdString()));
  if (!seedsText_.isEmpty()) text += seedsText_ + "\n";
  return text;
}

QString SimulationDialog::resultText() const { return result_->text(); }
