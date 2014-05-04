#include <boost/random.hpp>

#include "move.hpp"
#include "state.hpp"
#include "agent.hpp"

void Agent::start_pondering(const State state) {}
void Agent::stop_pondering() {}

Move Agent::decide(const State& state) {
  boost::optional<Move> move = state.random_move(generator);
  assert(move);
  return *move;
}

boost::future<Move> Agent::start_decision(const State state) {
  return boost::make_ready_future(decide(state));
}
void Agent::finalize_decision() {}
void Agent::abort_decision() {}

bool Agent::accept_draw(const State& state, Color color) {
  static boost::random::bernoulli_distribution<> distribution(0.1);
  return distribution(generator);
}

void Agent::idle() {
  abort_decision();
  stop_pondering();
}

void Agent::pause() {}
void Agent::resume() {}
