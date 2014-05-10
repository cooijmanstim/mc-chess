#pragma once

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_USES_MOVE
#include <boost/thread/future.hpp>

#include "state.hpp"

class Agent {
public:
  // ponder the given state
  virtual void start_pondering(const State state) = 0;
  virtual void stop_pondering() = 0;

  // decide synchronously
  virtual Move decide(const State& state) = 0;

  // decide asynchronously
  virtual boost::future<Move> start_decision(const State state) = 0;
  // tell the decision process to make up its mind
  virtual void finalize_decision() = 0;
  // tell the decision process to nevermind
  virtual void abort_decision() = 0;

  // whether the agent would draw in this state if it were playing color
  virtual bool accept_draw(const State& state, const Color color) = 0;

  // stop consuming cpu time and don't worry about saving state
  virtual void idle() = 0;

  // stop consuming cpu time temporarily, save state
  virtual void pause() = 0;

  // resume after pause()
  virtual void resume() = 0;
};
