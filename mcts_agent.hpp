#pragma once

#include <boost/random.hpp>

#include "agent.hpp"
#include "sometimes.hpp"
#include <boost/noncopyable.hpp>

#include "mcts.hpp"

class MCTSAgent : Agent, boost::noncopyable {
  boost::mt19937 generator;
  boost::thread_group ponderers;

  bool do_ponder;
  bool do_terminate;
  boost::optional<State> state;
  mcts::Graph graph;

  bool pending_change;
  boost::barrier barrier_before_change;
  boost::barrier barrier_after_change;

public:
  MCTSAgent(unsigned nponderers);
  ~MCTSAgent();

  // to safely communicate with ponderers
  void between_ponderings(std::function<void()> change);
  void perform_pondering(std::function<void()> pondering);

  void set_state(State state);
  void advance_state(Move move);

  void start_pondering();
  void stop_pondering();
  void ponder(boost::mt19937 generator);

  Move decide();
  boost::future<Move> start_decision();
  void finalize_decision();
  void abort_decision();

  bool accept_draw(Color color);

  void idle();
  void pause();
  void resume();
};
