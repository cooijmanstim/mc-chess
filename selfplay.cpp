#include <ios>

#include "mcts_agent.hpp"

#define fmt boost::format

int main(int argc, char* argv[]) {
  unsigned time_budget = 300;
  State state;
  MCTSAgent agent(2);
  agent.set_state(state);
  while (!state.game_over()) {
    auto decision = agent.start_decision(time_budget);
    boost::optional<Move> move = decision.get();
    agent.advance_state(*move);
  }
}
