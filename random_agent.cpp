#include "random_agent.hpp"

void RandomAgent::start_pondering(const State state) {}
void RandomAgent::stop_pondering() {}

Move RandomAgent::decide(const State& state) {
  boost::optional<Move> move = state.random_move(generator);
  assert(move);
  return *move;
}

boost::future<Move> RandomAgent::start_decision(const State state) {
  return boost::make_ready_future(decide(state));
}
void RandomAgent::finalize_decision() {}
void RandomAgent::abort_decision() {}

bool RandomAgent::accept_draw(const State& state, Color color) {
  static boost::random::bernoulli_distribution<> distribution(0.1);
  return distribution(generator);
}

void RandomAgent::idle() {
  abort_decision();
  stop_pondering();
}

void RandomAgent::pause() {}
void RandomAgent::resume() {}
