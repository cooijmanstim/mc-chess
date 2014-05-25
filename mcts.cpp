#include "mcts.hpp"

using namespace mcts;

Node::Node() {
}

const double
  loss_value = 0,
  draw_value = 0.5,
  win_value = 1;

Result Node::invert_result(Result result) {
  return 1 - result;
}

// use this to instantiate.  it ensures that there is always a shared_ptr
// pointing to the newly created instance, so that shared_from_this() will
// work.
FarNode Node::create(FarNode parent, boost::optional<Move> move, const State& state) {
  FarNode node = FarNode(new Node());
  node->parent = parent;
  node->state_hash = state.hash;
  node->last_move = move;
  node->last_player = colors::opposite(state.us);
  node->unexplored_moves = state.moves();
  node->total_result = 0;
  node->nvisits = 0;
  return node;
}

FarNode Node::add_child(Move move, const State& state) {
  FarNode child = create(this->shared_from_this(), move, state);
  auto position = std::find(this->unexplored_moves.begin(), this->unexplored_moves.end(), move);
  if (position != this->unexplored_moves.end())
    this->unexplored_moves.erase(position);
  this->children.push_back(child);
  return child;
}

void Node::sample(State state, boost::mt19937& generator) {
  assert(state.hash == this->state_hash);
  FarNode node = this->shared_from_this();
  node = node->select(state);
  node = node->expand(state, generator);
  double result = node->rollout(state, generator);
  node->backprop(result);
}

FarNode Node::select(State& state) {
  if (unexplored_moves.empty() && !children.empty()) {
    FarNode child = select_by(uct_score);
    state.make_move(*child->last_move);
    return child->select(state);
  } else {
    return this->shared_from_this();
  }
}

FarNode Node::expand(State& state, boost::mt19937& generator) {
  if (!unexplored_moves.empty()) {
    boost::uniform_int<> distribution(0, unexplored_moves.size() - 1);
    Move move = unexplored_moves.at(distribution(generator));
    state.make_move(move);
    return add_child(move, state);
  } else {
    return this->shared_from_this();
  }
}

Result Node::rollout(State& state, boost::mt19937& generator) {
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  State initial_state(state);
  std::vector<Move> move_history; // for debugging
#endif
  while (true) {
    if (state.drawn_by_50())
      return draw_value;
    boost::optional<Move> move = state.random_move(generator);
    if (!move)
      break;
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
    move_history.push_back(*move);
#endif
    state.make_move(*move);
  }
  boost::optional<Color> winner = state.winner();
  if (!winner)
    return draw_value;
  return *winner == last_player ? win_value : loss_value;
}

void Node::backprop(Result result) {
  FarNode node = shared_from_this();
  do {
    node->update(result);
    result = invert_result(result);
    node = node->parent;
  } while (node);
}

void Node::update(Result result) {
  total_result += result;
  nvisits++;
}

double Node::winrate(FarNode child) {
  assert(child->nvisits > 0);
  return child->total_result / child->nvisits;
}

double Node::uct_score(FarNode child) {
  assert(child->parent);
  assert(child->nvisits > 0);
  return winrate(child) + sqrt(2 * log(child->parent->nvisits) / child->nvisits);
}

double Node::most_visited(FarNode child) {
  return child->nvisits;
}

FarNode Node::select_by(std::function<double(FarNode)> key) {
  double best_value;
  FarNode best_child;
  for (FarNode curr_child: children) {
    double curr_value = key(curr_child);
    if (!best_child || best_value < curr_value) {
      best_value = curr_value;
      best_child = curr_child;
    }
  }
  return best_child;
}

Move Node::best_move() {
  FarNode best_child = select_by(most_visited);
  assert(best_child->last_move);
  return *best_child->last_move;
}

void Node::print_evaluations() {
  for (FarNode child: children) {
    std::cout << *child->last_move << " " << child->nvisits << " " << winrate(child) << " " << uct_score(child) << std::endl;
  }
}
