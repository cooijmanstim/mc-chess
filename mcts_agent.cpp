#include "mcts_agent.hpp"
#include "mcts.hpp"

void MCTSAgent::start_pondering(const State state) {
  if (ponderers.empty())
    spawn_ponderers();
  ponder_state = state;
}
void MCTSAgent::stop_pondering() {
  ponder_state = boost::none;
}

void MCTSAgent::ponder(boost::mt19937 generator) {
  while (true) {
    State state(*ponder_state);
    FarNode node = get_ponder_node(state);
    // FIXME: make mcts threadsafe enough
    for (int i = 0; i < 100; i++)
      node->sample(state, generator);
  }
}

void MCTSAgent::get_ponder_node(State const& state) {
  
}

void MCTSAgent::spawn_ponderers() {
  const int PONDERER_COUNT = 2;
  for (int i = 0; i < PONDERER_COUNT; i++) {
    auto seed = generator();
    auto ponderer = boost::thread([this, seed](){
        boost::mt19937 generator(seed);
        ponder(generator);
      });
    ponderers.push_back(ponderer);
  }
}

Move MCTSAgent::decide(const State& state) {
  throw std::runtime_error("not implemented");
}

boost::future<Move> MCTSAgent::start_decision(const State state) {
  return boost::async([state, this]() {
      mcts::FarNode tree = mcts::Node::create(0, boost::none, state);
      for (int i = 0; i < 1e5; i++)
        tree->sample(state, generator);
      tree->print_evaluations();
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
