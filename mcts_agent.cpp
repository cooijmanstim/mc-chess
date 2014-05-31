#include "mcts_agent.hpp"
#include "mcts.hpp"

void MCTSAgent::start_pondering(const State state) {
  if (ponderers.size() == 0)
    spawn_ponderers();
  mcts::Node* tree = 0;
  boost::optional<PonderState> ponder_state = this->ponder_state.peek();
  if (ponder_state)
    tree = ponder_state->node;
  this->ponder_state = PonderState{state, get_node(state, tree)};
}

void MCTSAgent::stop_pondering() {
  ponder_state = boost::none;
}

void MCTSAgent::ponder(boost::mt19937 generator) {
  while (true) {
    PonderState ponder_state(*this->ponder_state);
    for (int i = 0; i < 100; i++)
      ponder_state.node->sample(ponder_state.state, generator);
  }
}

mcts::Node* MCTSAgent::get_node(State const& state, mcts::Node* tree) {
  if (tree) {
    for (int depth = 0; depth < 10; depth++) {
      mcts::Node* node = tree->find_node(state.hash, depth);
      if (node)
        return node;
    }
    delete tree;
  }
  return new mcts::Node(nullptr, boost::none, state);
}

void MCTSAgent::spawn_ponderers() {
  const int PONDERER_COUNT = 2;
  for (int i = 0; i < PONDERER_COUNT; i++) {
    auto seed = generator();
    ponderers.create_thread([this, seed](){
        boost::mt19937 generator(seed);
        ponder(generator);
      });
  }
}

Move MCTSAgent::decide(const State& state) {
  throw std::runtime_error("not implemented");
}

boost::future<Move> MCTSAgent::start_decision(const State state) {
  start_pondering(state);
  mcts::Node* node = (*ponder_state).node;
  return boost::async([=]() {
      boost::this_thread::sleep_for(boost::chrono::seconds(5));
      node->print_evaluations();
      return node->best_move();
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
