#include "util.hpp"
#include "state.hpp"
#include "move.hpp"
#include "notation.hpp"

#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <vector>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>

void print_backtrace() {
  const size_t max_size = 30;
  void* array[max_size];
  size_t size;

  size = backtrace(array, max_size);
  fprintf(stderr, "Backtrace:\n");
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}

void sleep_forever() {
  while (true)
    boost::this_thread::sleep_for(boost::chrono::hours(1000));
}

void dump_for_debug(State initial_state, std::vector<Move> moves) {
  std::cerr << "Begin debug dump" << std::endl;
  std::cerr << "Initial state: " << initial_state.dump_fen() << std::endl;
  std::cerr << "Moves: ";
  for (Move move: moves) {
    std::cerr << notation::coordinate::format(move) << " ";
  }
  std::cerr << std::endl;
  std::cerr << "End debug dump" << std::endl;
}

std::vector<std::string> words(std::string string) {
  boost::regex separator("\\s+");
  std::vector<std::string> words;
  boost::algorithm::split_regex(words, boost::algorithm::trim_copy(string), separator);
  return words;
}

