#include <ios>

#include "mcts_agent.hpp"

#define fmt boost::format

int main(int argc, char* argv[]) {
  boost::optional<std::string> path_to_storage;
  if (argc > 1)
    path_to_storage = std::string(argv[1]);

  unsigned time_budget = 10;
  State state;
  MCTSAgent agent(2);

  if (path_to_storage)
    agent.load_yourself(*path_to_storage);

  agent.set_state(state);
  while (!state.game_over()) {
    auto decision = agent.start_decision(time_budget);
    boost::optional<Move> move = decision.get();
    state.make_move(*move);
    agent.advance_state(*move);
  }
  
  if (path_to_storage)
    agent.save_yourself(*path_to_storage);
}
