#include "mcts.hpp"

using namespace mcts;

Node::Node(Node* parent, boost::optional<Move> move, const State& state)
  : parent(parent),
    children(nullptr),
    child_count(0),
    state_hash(state.hash),
    last_move(move),
    total_result(0),
    visit_count(0)
{
}

Node::~Node() {
  if (children && child_count > 0) {
    // this is thread-safe because children and child_count are constant
    // after the node is expanded.
    Node* end = children + child_count;
    for (Node *child = children; child < end; child++)
      child->~Node();
    ::operator delete(children);
  }
}

const double
  loss_value = 0,
  draw_value = 0.5,
  win_value = 1;

Result Node::invert_result(Result result) {
  return 1 - result;
}

Node* Node::find_node(Hash state_hash, unsigned depth) {
  if (this->state_hash == state_hash)
    return this;
  if (depth > 0) {
    if (children && child_count > 0) {
      Node* end = children + child_count;
      for (Node* child = children; child < end; child++) {
        Node* node = child->find_node(state_hash, depth - 1);
        if (node)
          return node;
      }
    }
  }
  return nullptr;
}

void Node::sample(State state, boost::mt19937& generator) {
  assert(state.hash == this->state_hash);
  Node* node = this;
  node = node->select(state);
  node = node->expand(state, generator);
  double result = node->rollout(state, generator);
  node->backprop(result);
}

Node* Node::select(State& state) {
  Node* node = this;
  while (node->children && node->child_count != 0) {
    node = node->select_by(uct_score);
    assert(node);
    state.make_move(*node->last_move);
  }
  return node;
}

Node* Node::expand(State& state, boost::mt19937& generator) {
  std::vector<Move> moves = state.moves();
  Node* children = static_cast<Node*>(::operator new(moves.size() * sizeof(Node)));
  for (size_t i = 0; i < moves.size(); i++) {
    // to avoid having to keep track of unexplored_moves, and to ensure the
    // children sequence is constant-size, construct all children right away.
    // TODO: doing this requires copying state.  if we're copying state anyway,
    // maybe might as well do a rollout for each of the children.
    State child_state(state);
    child_state.make_move(moves[i]);
    new(&children[i]) Node(this, moves[i], child_state);
  }
  this->child_count = moves.size();
  this->children = children;
  // select one of the children
  return select(state);
}

Result Node::rollout(State& state, boost::mt19937& generator) {
  Color initial_player = state.us;
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
  return *winner == initial_player ? loss_value : win_value;
}

void Node::backprop(Result result) {
  Node* node = this;
  do {
    node->update(result);
    result = invert_result(result);
    node = node->parent;
  } while (node);
}

void Node::update(Result result) {
  total_result += result;
  visit_count++;
}

double Node::winrate(Node* child) {
  if (child->visit_count == 0)
    return 0.5;
  return child->total_result / child->visit_count;
}

double Node::uct_score(Node* child) {
  if (child->visit_count == 0)
    return 1e6;
  assert(child->parent);
  return winrate(child) + sqrt(2 * log(child->parent->visit_count) / child->visit_count);
}

double Node::most_visited(Node* child) {
  return child->visit_count;
}

Node* Node::select_by(std::function<double(Node*)> key) {
  double best_value;
  Node* best_child = nullptr;
  if (children && child_count > 0) {
    Node* end = children + child_count;
    for (Node* curr_child = children; curr_child < end; curr_child++) {
      double curr_value = key(curr_child);
      if (!best_child || best_value < curr_value) {
        best_value = curr_value;
        best_child = curr_child;
      }
    }
  }
  return best_child;
}

Move Node::best_move() {
  Node* best_child = select_by(most_visited);
  assert(best_child);
  assert(best_child->last_move);
  return *best_child->last_move;
}

void Node::do_children(std::function<void(Node*)> f) {
  if (!children)
    return;
  Node* end = children + child_count;
  for (Node* curr_child = children; curr_child < end; curr_child++) {
    f(curr_child);
  }
}

void Node::print_evaluations() {
  do_children([](Node* child) {
    std::cout << *child->last_move << " " << child->visit_count << " " << winrate(child) << " " << uct_score(child) << std::endl;
  });
}
