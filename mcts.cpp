#include "mcts.hpp"

using namespace mcts;

const int loss_value = 0,
          draw_value = 1,
           win_value = 2;
const int result_factor = 2;

Node::Node() {
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
  float result = node->rollout(state, generator);
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

#define ROLLOUT_TRACK_HISTORY 1
int Node::rollout(State& state, boost::mt19937& generator) {
#if ROLLOUT_TRACK_HISTORY
  State initial_state(state);
  std::vector<Move> move_history; // for debugging
#endif
  while (true) {
    if (state.drawn_by_50())
      return draw_value;
    boost::optional<Move> move = state.random_move(generator);
    if (!move)
      break;
#if ROLLOUT_TRACK_HISTORY
    move_history.push_back(*move);
#endif
    state.make_move(*move);
  }
  boost::optional<Color> winner = state.winner();
  if (!winner)
    return draw_value;
  return *winner == last_player ? win_value : loss_value;
}

void Node::backprop(int result) {
  update(result);
  if (parent)
    parent->update(1 - result);
}

void Node::update(int result) {
  total_result += result;
  nvisits++;
}

float Node::uct_score(FarNode child) {
  assert(child->parent);
  return float(child->total_result) / result_factor / child->nvisits + sqrt(2 * log(child->parent->nvisits) / child->nvisits);
}

float Node::most_visited(FarNode child) {
  return child->nvisits;
}

FarNode Node::select_by(std::function<float(FarNode)> key) {
  float best_value;
  FarNode best_child;
  for (FarNode curr_child: children) {
    float curr_value = key(curr_child);
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
