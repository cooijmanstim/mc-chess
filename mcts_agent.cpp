#include "mcts_agent.hpp"
#include "mcts.hpp"

void MCTSAgent::start_pondering(const State state) {}
void MCTSAgent::stop_pondering() {}

Move MCTSAgent::decide(const State& state) {
  throw std::runtime_error("not implemented");
}

boost::future<Move> MCTSAgent::start_decision(const State state) {
  return boost::async([&]() {
      mcts::FarNode tree = mcts::Node::create(0, boost::none, state);
      for (int i = 0; i < 1e3; i++) {
        tree->sample(state, generator);
      }
      return tree->best_move();
    });
}
void MCTSAgent::finalize_decision() {
  // FIXME
}
void MCTSAgent::abort_decision() {
  // FIXME
}

bool MCTSAgent::accept_draw(const State& state, Color color) {
  static boost::random::bernoulli_distribution<> distribution(0.1);
  return distribution(generator);
}

void MCTSAgent::idle() {
  abort_decision();
  stop_pondering();
}

void MCTSAgent::pause() {}
void MCTSAgent::resume() {}
