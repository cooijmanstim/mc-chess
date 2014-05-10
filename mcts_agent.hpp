#pragma once

#include <boost/random.hpp>

#include "agent.hpp"

class MCTSAgent : Agent {
  boost::mt19937 generator;

public:
  void start_pondering(const State state);
  void stop_pondering();
  Move decide(const State& state);
  boost::future<Move> start_decision(const State state);
  void finalize_decision();
  void abort_decision();
  bool accept_draw(const State& state, const Color color);
  void idle();
  void pause();
  void resume();
};
