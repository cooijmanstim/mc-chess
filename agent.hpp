#pragma once

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_USES_MOVE
#include <boost/thread/future.hpp>

#include "state.hpp"

class Agent {
public:
  virtual void set_state(State state) = 0;
  virtual void advance_state(Move move) = 0;

  // ponder the current state
  virtual void start_pondering() = 0;
  virtual void stop_pondering() = 0;

  // decide synchronously
  virtual Move decide() = 0;

  // decide asynchronously (time_budget in seconds)
  virtual boost::future<Move> start_decision(size_t time_budget) = 0;
  // tell the decision process to make up its mind
  virtual void finalize_decision() = 0;
  // tell the decision process to nevermind
  virtual void abort_decision() = 0;

  // whether the agent would draw in the current state if it were playing color
  virtual bool accept_draw(Color color) = 0;

  // stop consuming cpu time and don't worry about saving state
  virtual void idle() = 0;

  // stop consuming cpu time temporarily, save state
  virtual void pause() = 0;

  // resume after pause()
  virtual void resume() = 0;
};
