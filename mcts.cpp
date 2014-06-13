#include "mcts.hpp"
#include "notation.hpp"

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
    // NOTE: not entirely true anymore, but still thread-safe.  MCTSAgent
    // uses a persistent tree and occasionally destroys parts of it, but
    // always makes sure that nobody else is working on the tree at the
    // same time.
    Node* end = children + child_count;
    for (Node *child = children; child < end; child++)
      child->~Node();
    ::operator delete(children);
  }
}

Node::Node(Node&& that) {
  // the move idiom doesn't really fit what we're trying to do here.  in
  // destroy_parent_and_siblings(), we want to destroy the node's siblings
  // and free their memory, but the parent's children are all stored in one
  // contiguous block of memory, so we can't free selectively.  instead, we
  // reallocate memory for this node elsewhere and move its resources there.
  // the part that doesn't fit the move idiom is that the parent is still
  // pointing to the old memory location for its children even though one
  // of its children has moved.  this is the behavior we need, because then
  // we can destroy the parent and have it take the siblings down with it.
  // all we need to do is ensure that.children no longer refers to the list
  // of this node's children (because we want to keep it and not have it
  // destroyed).
  // tl;dr: behavior of the move constructor is tailored to
  // destroy_parent_and_siblings() and may not fit other uses.
  this->parent = nullptr;
  this->children = that.children;
  this->child_count = that.child_count;
  this->state_hash = that.state_hash;
  this->last_move = that.last_move;
  this->total_result = that.total_result;
  this->visit_count = that.visit_count;

  that.child_count = 0;
  that.children = nullptr;
  do_children([this](Node* child) {
      child->parent = this;
    });
}

const double
  loss_value = 0,
  draw_value = 0.5,
  win_value = 1;

Result Node::invert_result(Result result) {
  return 1 - result;
}

Node* Node::get_child(Move move) {
  if (children && child_count > 0) {
    Node* end = children + child_count;
    for (Node *child = children; child < end; child++)
      if (child->last_move == move)
        return child;
  }
  return nullptr;
}

Node* Node::destroy_parent_and_siblings() {
  Node* that = new Node(std::move(*this));
  assert(parent);
  delete parent;
  return that;
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
  while (node->children && node->child_count > 0) {
    node = node->select_by(uct_score);
    assert(node);
    state.make_move(*node->last_move);
  }
  return node;
}

Node* Node::expand(State& state, boost::mt19937& generator) {
  // TODO: computation of legal_moves() involves making the moves and then
  // unmaking them again.  in the loop below we do the same.  maybe combine.
  std::vector<Move> moves;
  moves::legal_moves(moves, state);

  Node* children = static_cast<Node*>(::operator new(moves.size() * sizeof(Node)));
  for (size_t i = 0; i < moves.size(); i++) {
    // to avoid having to keep track of unexplored_moves, and to ensure the
    // children sequence is constant-size, construct all children right away.
    Undo undo = state.make_move(moves[i]);
    new(&children[i]) Node(this, moves[i], state);
    state.unmake_move(undo);
  }
  this->child_count = moves.size();
  this->children = children;
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  do_children([](Node* child) {
      assert(child->last_move);
    });
#endif
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
    boost::optional<Move> move = moves::random_move(state, generator);
    //boost::optional<Move> move = moves::make_random_legal_move(state, generator);
    if (!move)
      break;
    state.make_move(*move);
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
    move_history.push_back(*move);
    state.require_consistent();
#endif
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

void Node::print_statistics(std::ostream& os) {
  do_children([&](Node* child) {
    os << *child->last_move << " " << child->visit_count << " " << winrate(child) << " " << uct_score(child) << std::endl;
  });
  os << visit_count << " " << winrate(this) << std::endl;
}

void Node::print_principal_variation(std::ostream& os) {
  Node* child = select_by(most_visited);
  if (!child)
    return;
  os << *child->last_move << " " << child->visit_count << " " << winrate(child) << std::endl;
  child->print_principal_variation(os);
}

void Node::graphviz(std::ostream& os) {
  auto hash_as_id = [this](Hash hash) {
    return str(boost::format("\"%1$#8x\"") % hash);
  };
  os << hash_as_id(state_hash)
     << "[label=\"{"
     << (last_move ? notation::coordinate::format(*last_move) : "-")
     << " | "
     << winrate(this) << " (" << total_result << "/" << visit_count << ") "
     << " | "
     << (parent ? str(boost::format("%f") % uct_score(this)) : "-")
     << "}\"];"
     << std::endl;
  do_children([&](Node* child) {
      // to keep the graph small
      if (child->visit_count == 0)
        return;
      child->graphviz(os);
      os << hash_as_id(state_hash) << " -> " << hash_as_id(child->state_hash) << ";" << std::endl;
    });
}
