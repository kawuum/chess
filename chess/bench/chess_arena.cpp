// chess_arena: a long run of all AIs against each other, to find crashes and
// to compare their strength and speed.
//
//   chess_arena [--minutes M] [--ais "Name 1,Name 2"]
//
// Every pairing - each AI against every other AI and against itself - plays
// games for M minutes (default 30), colors swapping after every game, with
// random seeds (printed, so a pairing can be played again). For every
// pairing it reports wins, losses and draws (also by color), the number of
// games and games per minute, the average length, and the memory of the
// process (peak, and at the start and end of the pairing: growth would hint
// at a leak).
//
// Every pairing runs in a process of its own (fork). If it crashes, the
// arena notes "CRASH" with the signal and goes on with the next pairing;
// an exception (e.g. an illegal move) is noted as "ERROR". Exit code 1 if
// anything crashed or failed, else 0.
//
// Run it in the release build. 4 AIs = 10 pairings = 5 hours at 30 minutes.
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "../ai/match.hpp"

namespace {

// Resident memory of this process right now, in KB (VmRSS).
long resident_kb() {
  std::ifstream in("/proc/self/status");
  std::string line;
  while (std::getline(in, line)) {
    if (line.rfind("VmRSS:", 0) == 0) return std::stol(line.substr(6));
  }
  return 0;
}

std::string now_text() {
  std::time_t t = std::time(nullptr);
  char text[32];
  std::strftime(text, sizeof text, "%Y-%m-%d %H:%M", std::localtime(&t));
  return text;
}

// What a pairing process reports back to the arena, as one line of numbers.
struct PairingResult {
  match_stats stats;
  uint32_t first_seed = 0, second_seed = 0;
  double seconds = 0;
  long rss_start_kb = 0, rss_end_kb = 0;
};

std::string to_line(const PairingResult& r) {
  std::ostringstream out;
  const match_stats& s = r.stats;
  out << s.games << ' ' << s.first_wins << ' ' << s.second_wins << ' '
      << s.white_wins << ' ' << s.black_wins << ' ' << s.draws << ' '
      << s.unfinished << ' ' << s.total_plies << ' ' << r.first_seed << ' '
      << r.second_seed << ' ' << r.seconds << ' ' << r.rss_start_kb << ' '
      << r.rss_end_kb;
  for (int count : s.draws_by) out << ' ' << count;
  return out.str();
}

bool from_line(const std::string& line, PairingResult& r) {
  std::istringstream in(line);
  match_stats& s = r.stats;
  in >> s.games >> s.first_wins >> s.second_wins >> s.white_wins >>
      s.black_wins >> s.draws >> s.unfinished >> s.total_plies >>
      r.first_seed >> r.second_seed >> r.seconds >> r.rss_start_kb >>
      r.rss_end_kb;
  for (int& count : s.draws_by) in >> count;
  return static_cast<bool>(in);
}

// Runs in the child process: plays first against second until the time is
// up, then writes the result line to fd.
int play_pairing(const ai_info& first, const ai_info& second, double minutes,
                 int fd) {
  PairingResult r;
  std::random_device device;
  r.first_seed = device();
  r.second_seed = device();
  match_settings settings;
  settings.games = INT_MAX;  // stopped by the time limit below
  settings.swap_colors = true;
  settings.first_seed = r.first_seed;
  settings.second_seed = r.second_seed;

  r.rss_start_kb = resident_kb();
  const auto start = std::chrono::steady_clock::now();
  auto elapsed = [&start] {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                         start)
        .count();
  };
  try {
    r.stats = play_match(first, second, settings, [&](const match_stats&) {
      return elapsed() < minutes * 60;
    });
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "error in %s against %s: %s\n", first.name.c_str(),
                 second.name.c_str(), ex.what());
    return 3;
  }
  r.seconds = elapsed();
  r.rss_end_kb = resident_kb();
  const std::string line = to_line(r) + "\n";
  if (write(fd, line.data(), line.size()) != (ssize_t)line.size()) return 4;
  return 0;
}

std::vector<const ai_info*> select_ais(const std::string& names) {
  std::vector<const ai_info*> selected;
  for (const ai_info& info : available_ais()) {
    if (names.empty() ||
        ("," + names + ",").find("," + info.name + ",") != std::string::npos) {
      selected.push_back(&info);
    }
  }
  return selected;
}

}  // namespace

int main(int argc, char** argv) {
  double minutes = 30;
  std::string names;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--minutes" && i + 1 < argc) {
      minutes = std::atof(argv[++i]);
    } else if (arg == "--ais" && i + 1 < argc) {
      names = argv[++i];
    } else {
      std::fprintf(stderr,
                   "usage: chess_arena [--minutes M] [--ais \"Name,Name\"]\n");
      return 2;
    }
  }
#ifndef NDEBUG
  std::printf("WARNING: debug build, games are much slower.\n");
#endif
  const std::vector<const ai_info*> ais = select_ais(names);
  std::vector<std::pair<const ai_info*, const ai_info*>> pairings;
  for (size_t i = 0; i < ais.size(); ++i) {
    for (size_t j = i; j < ais.size(); ++j) {
      pairings.emplace_back(ais[i], ais[j]);
    }
  }
  std::printf("chess_arena %s: %zu AIs, %zu pairings x %.1f min = %.1f h\n\n",
              now_text().c_str(), ais.size(), pairings.size(), minutes,
              pairings.size() * minutes / 60);
  std::printf("%-14s %-14s %6s %6s %6s %6s | %6s %6s | %5s %5s %6s %9s %9s\n",
              "first", "second", "games", "1 wins", "2 wins", "draws", "white",
              "black", "unfin", "plies", "g/min", "peak MB", "RSS MB");
  std::fflush(stdout);

  int failures = 0;
  for (const auto& pairing : pairings) {
    int pipe_fds[2];
    if (pipe(pipe_fds) != 0) {
      std::perror("pipe");
      return 2;
    }
    std::fflush(stdout);
    const pid_t child = fork();
    if (child == 0) {  // the pairing process
      close(pipe_fds[0]);
      _exit(
          play_pairing(*pairing.first, *pairing.second, minutes, pipe_fds[1]));
    }
    close(pipe_fds[1]);
    std::string line;
    char buffer[512];
    ssize_t n;
    while ((n = read(pipe_fds[0], buffer, sizeof buffer)) > 0) {
      line.append(buffer, n);
    }
    close(pipe_fds[0]);
    int status = 0;
    struct rusage usage{};
    wait4(child, &status, 0, &usage);

    std::printf("%-14s %-14s ", pairing.first->name.c_str(),
                pairing.second->name.c_str());
    PairingResult r;
    if (WIFSIGNALED(status)) {
      ++failures;
      std::printf("CRASH: signal %d (%s)\n", WTERMSIG(status),
                  strsignal(WTERMSIG(status)));
    } else if (WEXITSTATUS(status) != 0 || !from_line(line, r)) {
      ++failures;
      std::printf("ERROR: exit code %d (see message above)\n",
                  WEXITSTATUS(status));
    } else {
      const match_stats& s = r.stats;
      std::printf(
          "%6d %6d %6d %6d | %6d %6d | %5d %5lld %6.1f %9.1f %4.1f->%4.1f\n",
          s.games, s.first_wins, s.second_wins, s.draws, s.white_wins,
          s.black_wins, s.unfinished, s.games > 0 ? s.total_plies / s.games : 0,
          s.games / (r.seconds / 60), usage.ru_maxrss / 1024.0,
          r.rss_start_kb / 1024.0, r.rss_end_kb / 1024.0);
      std::printf(
          "%30s draws: stalemate %d, repetition %d, 50 moves %d, "
          "material %d; seeds %u / %u\n",
          "", s.draws_by[STALEMATE], s.draws_by[THREEFOLD_REPETITION],
          s.draws_by[FIFTY_MOVE_RULE], s.draws_by[INSUFFICIENT_MATERIAL],
          r.first_seed, r.second_seed);
    }
    std::fflush(stdout);
  }

  std::printf(
      "\n%s: %s\n", now_text().c_str(),
      failures == 0 ? "no crashes, no errors" : "FAILURES, see the table");
  return failures == 0 ? 0 : 1;
}
