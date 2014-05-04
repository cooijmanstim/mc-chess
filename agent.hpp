#include <boost/random.hpp>

#include "move.hpp"
#include "state.hpp"

class Agent {
  boost::mt19937 generator;

  // ponder the given state
  void start_pondering(const State state);
  void stop_pondering();

  // decide synchronously
  Move decide(const State& state);

  // decide asynchronously
  boost::future<Move> start_decision(const State state);
  // tell the decision process to make up its mind
  void finalize_decision();
  // tell the decision process to nevermind
  void abort_decision();

  // whether the agent would draw in this state if it were playing color
  boolean accept_draw(const State& state, const Color color);

  // stop consuming cpu time and don't worry about saving state
  void idle();

  // stop consuming cpu time temporarily, save state
  void pause();

  // resume after pause()
  void resume();
}
