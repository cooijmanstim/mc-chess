#pragma once

#include <boost/random.hpp>

#include "agent.hpp"
#include "sometimes.hpp"
#include "mcts.hpp"

struct PonderState {
  State state;
  mcts::Node* node;
};

class MCTSAgent : Agent {
  boost::mt19937 generator;
  boost::thread_group ponderers;
  Sometimes<PonderState> ponder_state;

public:
  void start_pondering(const State state);
  void stop_pondering();
  mcts::Node* get_node(State const& state, mcts::Node* tree);
  void spawn_ponderers();
  void ponder(boost::mt19937 generator);
  Move decide(const State& state);
  boost::future<Move> start_decision(const State state);
  void finalize_decision();
  void abort_decision();
  bool accept_draw(const State& state, const Color color);
  void idle();
  void pause();
  void resume();
};
