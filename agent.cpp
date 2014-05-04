#include <boost/random.hpp>

#include "move.hpp"
#include "state.hpp"

class Agent {
  boost::mt19937 generator;

  void start_pondering(const State state) {}
  void stop_pondering() {}

  Move decide(const State& state) {
    return boost::make_future(state.random_move(generator));
  }

  boost::future<Move> start_decision(const State state) {
    return boost::make_future(state.random_move(generator));
  }
  void finalize_decision();
  void abort_decision();

  boolean accept_draw(const State& state, Color color) {
    static bernoulli_distribution distribution(0.1);
    return distribution(generator);
  }

  void idle() {
    abort_decision();
    stop_pondering();
  }

  void pause() {}
  void resume() {}
}
