// chess_bench: speed measurement for the engine.
//
//   chess_bench                     run the benchmark suite
//   chess_bench --save FILE         run it and save the results as baseline
//   chess_bench --compare FILE      run it and compare with a saved baseline
//   chess_bench perft DEPTH [FEN]   count one position (start position if no
//                                   FEN) and show the time
//   chess_bench divide DEPTH [FEN]  like perft, split by the first move
//
// Only meaningful in a release build. The suite also checks the node counts:
// a wrong count ends the program with exit code 1, because a fast but wrong
// move generator is worth nothing.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../engine/perft.hpp"

namespace {

const char* const kStartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct BenchCase {
  const char* name;
  const char* fen;
  int depth;
  uint64_t nodes;  // reference count, see tests/perft_test.cpp
};

// Each case covers different rules: castling, en passant, promotions, pins.
// The depths only decide how long one run takes; the total time of the suite
// is set by kSecondsPerCase below.
const std::vector<BenchCase> kSuite = {
    {"start", kStartFen, 4, 197281},
    {"kiwipete",
     "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3,
     97862},
    {"position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624},
    {"position4",
     "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4,
     422333},
    {"position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
     3, 62379},
    {"position6",
     "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
     3, 89890},
};

// Each case runs again and again until kSecondsPerCase are used up (at least
// kMinRuns times); the fastest run counts. That filters out runs disturbed by
// other programs. With a time budget instead of a fixed number of runs the
// suite always takes about 6 x 8 s, however fast the engine gets: a faster
// engine simply gets more runs, which makes the best time even more reliable.
// Note: on a laptop, a long full load can lower the clock (heat). Start the
// benchmark on an otherwise idle machine, not right after another heavy run.
constexpr int kMinRuns = 2;
constexpr int kMaxRuns = 100000;
constexpr double kSecondsPerCase = 8.0;

struct Result {
  std::string name;
  int depth;
  uint64_t nodes;
  double seconds;  // best run
  int runs;
};

double now_seconds() {
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

Result run_case(const BenchCase& c) {
  engine e;
  e.new_game(c.fen);
  double best = 1e100, total = 0;
  int runs = 0;
  for (; runs < kMaxRuns && (runs < kMinRuns || total < kSecondsPerCase);
       ++runs) {
    double t0 = now_seconds();
    uint64_t nodes = perft(e, c.depth);
    double t = now_seconds() - t0;
    if (nodes != c.nodes) {
      std::fprintf(stderr,
                   "WRONG node count for %s depth %d: %llu, expected %llu\n",
                   c.name, c.depth, (unsigned long long)nodes,
                   (unsigned long long)c.nodes);
      std::exit(1);
    }
    best = std::min(best, t);
    total += t;
  }
  return {c.name, c.depth, c.nodes, best, runs};
}

// Baseline file: one line per case "name depth nodes seconds", # = comment.
std::map<std::string, double> read_baseline(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    std::fprintf(stderr, "cannot read baseline %s\n", path.c_str());
    std::exit(2);
  }
  std::map<std::string, double> seconds;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream fields(line);
    std::string name;
    int depth;
    uint64_t nodes;
    double s;
    if (fields >> name >> depth >> nodes >> s) seconds[name] = s;
  }
  return seconds;
}

std::string cpu_model() {
  std::ifstream in("/proc/cpuinfo");
  std::string line;
  while (std::getline(in, line)) {
    if (line.rfind("model name", 0) == 0) {
      return line.substr(line.find(':') + 2);
    }
  }
  return "unknown CPU";
}

void save_baseline(const std::string& path,
                   const std::vector<Result>& results) {
  std::ofstream out(path);
  std::time_t t = std::time(nullptr);
  char date[32];
  std::strftime(date, sizeof date, "%Y-%m-%d", std::localtime(&t));
  out << "# chess_bench baseline, " << date << ", " << cpu_model() << "\n"
      << "# name depth nodes best_seconds\n";
  for (const Result& r : results) {
    char line[128];
    std::snprintf(line, sizeof line, "%s %d %llu %.6f\n", r.name.c_str(),
                  r.depth, (unsigned long long)r.nodes, r.seconds);
    out << line;
  }
}

int run_suite(const std::string& save_path, const std::string& compare_path) {
#ifndef NDEBUG
  std::printf("WARNING: debug build, the times are not meaningful.\n\n");
#endif
  std::map<std::string, double> baseline;
  if (!compare_path.empty()) baseline = read_baseline(compare_path);

  std::printf("%-10s %5s %9s %5s %9s %11s", "position", "depth", "nodes",
              "runs", "best", "nodes/s");
  if (!baseline.empty()) std::printf(" %9s %8s", "baseline", "change");
  std::printf("\n");

  std::vector<Result> results;
  double total_s = 0;
  uint64_t total_nodes = 0;
  // Overall change = geometric mean of the per-position ratios, so that every
  // position counts the same, however long it runs.
  double log_ratio_sum = 0;
  int compared = 0;
  for (const BenchCase& c : kSuite) {
    Result r = run_case(c);
    results.push_back(r);
    total_s += r.seconds;
    total_nodes += r.nodes;
    std::printf("%-10s %5d %9llu %5d %8.3fs %11.0f", r.name.c_str(), r.depth,
                (unsigned long long)r.nodes, r.runs, r.seconds,
                r.nodes / r.seconds);
    auto base = baseline.find(r.name);
    if (base != baseline.end()) {
      double ratio = r.seconds / base->second;
      log_ratio_sum += std::log(ratio);
      ++compared;
      std::printf(" %8.3fs %+7.1f%%", base->second, (ratio - 1) * 100);
    }
    std::printf("\n");
    std::fflush(stdout);
  }
  std::printf("%-10s %5s %9llu %5s %8.3fs %11.0f\n", "total", "",
              (unsigned long long)total_nodes, "", total_s,
              total_nodes / total_s);
  if (compared > 0) {
    double ratio = std::exp(log_ratio_sum / compared);  // geometric mean
    std::printf(
        "\nchange vs. baseline (geometric mean of all positions): %+.1f%%, "
        "speedup x%.2f\n(negative = faster; below 3%% is measurement noise)\n",
        (ratio - 1) * 100, 1 / ratio);
  }

  if (!save_path.empty()) {
    save_baseline(save_path, results);
    std::printf("baseline saved to %s\n", save_path.c_str());
  }
  return 0;
}

int run_perft(int depth, const std::string& fen, bool divide) {
  engine e;
  e.new_game(fen);
  double t0 = now_seconds();
  uint64_t nodes = 0;
  if (divide) {
    for (const auto& entry : perft_divide(e, depth)) {
      std::printf("%s: %llu\n", entry.first.c_str(),
                  (unsigned long long)entry.second);
      nodes += entry.second;
    }
  } else {
    nodes = perft(e, depth);
  }
  double t = now_seconds() - t0;
  std::printf("nodes %llu, time %.3f s, %.0f nodes/s\n",
              (unsigned long long)nodes, t, nodes / t);
  return 0;
}

void usage() {
  std::fprintf(stderr,
               "usage: chess_bench [--save FILE | --compare FILE]\n"
               "       chess_bench perft DEPTH [FEN]\n"
               "       chess_bench divide DEPTH [FEN]\n");
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc);
  try {
    if (!args.empty() && (args[0] == "perft" || args[0] == "divide")) {
      if (args.size() < 2) {
        usage();
        return 2;
      }
      int depth = std::stoi(args[1]);
      // The FEN may come as one argument or as several words.
      std::string fen;
      for (size_t i = 2; i < args.size(); ++i)
        fen += (i > 2 ? " " : "") + args[i];
      return run_perft(depth, fen.empty() ? kStartFen : fen,
                       args[0] == "divide");
    }
    std::string save, compare;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i] == "--save" && i + 1 < args.size()) {
        save = args[++i];
      } else if (args[i] == "--compare" && i + 1 < args.size()) {
        compare = args[++i];
      } else {
        usage();
        return 2;
      }
    }
    return run_suite(save, compare);
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "error: %s\n", ex.what());
    return 2;
  }
}
