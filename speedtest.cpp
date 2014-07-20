#include <ios>

#include "move.hpp"
#include "state.hpp"
#include "mcts_agent.hpp"

int main(int argc, char* argv[]) {
  std::cout << "cumulative sampling durations for initial state:" << std::endl;
  boost::mt19937 generator;
  State state;
  mcts::Graph graph;
  auto then = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= 1e4; i++) {
    graph.sample(state, generator);
    if (i % 1000 == 0) {
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - then);
      std::cout << i << " " << duration.count() << std::endl;
    }
  }
}
