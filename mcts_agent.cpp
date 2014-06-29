#include "mcts_agent.hpp"
#include "mcts.hpp"

MCTSAgent::MCTSAgent(unsigned nponderers)
  : pending_change(false),
    barrier_before_change(nponderers + 1),
    barrier_after_change(nponderers + 1),
    do_ponder(false),
    do_terminate(false)
{
  for (unsigned i = 0; i < nponderers; i++) {
    auto seed = generator();
    ponderers.create_thread([this, seed](){
        boost::mt19937 generator(seed);
        ponder(generator);
      });
  }
}

MCTSAgent::~MCTSAgent() {
  do_terminate = true;
  ponderers.join_all();
}

void MCTSAgent::set_state(State state) {
  if (state == this->state)
    return;
  between_ponderings([this, state]() {
      this->state = state;
    });
}

void MCTSAgent::advance_state(Move move) {
  assert(this->state);
  between_ponderings([this, move]() {
      this->state->make_move(move);
    });
}

void MCTSAgent::start_pondering() {
  between_ponderings([this]() {
      do_ponder = true;
    });
}

void MCTSAgent::stop_pondering() {
  between_ponderings([this]() {
      do_ponder = false;
    });
}

void MCTSAgent::ponder(boost::mt19937 generator) {
  while (!do_terminate) {
    perform_pondering([this, &generator]() {
        graph.sample(*state, generator);
      });
  }
}

Move MCTSAgent::decide() {
  throw std::runtime_error("not implemented");
}

boost::future<Move> MCTSAgent::start_decision() {
  start_pondering();
  return boost::async([this]() {
      boost::this_thread::sleep_for(boost::chrono::seconds(30));
      std::cout << "mcts result for state: " << std::endl;
      std::cout << *state << std::endl;
      std::cout << "candidate moves: " << std::endl;
      graph.print_statistics(std::cout, *state);
      std::cout << "principal variation: " << std::endl;
      graph.print_principal_variation(std::cout, *state);
      boost::optional<Move> move = graph.principal_move(*state);
      if (!move)
        throw std::runtime_error("no moves in root state");
      return *move;
    });
}
void MCTSAgent::finalize_decision() {
  // FIXME
}
void MCTSAgent::abort_decision() {
  // FIXME
}

bool MCTSAgent::accept_draw(Color color) {
  static boost::random::bernoulli_distribution<> distribution(0.1);
  return distribution(generator);
}

void MCTSAgent::idle() {
  abort_decision();
  stop_pondering();
}

void MCTSAgent::pause() {
  stop_pondering();
}
void MCTSAgent::resume() {
  start_pondering();
}
