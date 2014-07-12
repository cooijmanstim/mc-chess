#include "mcts.hpp"
#include "notation.hpp"

#include <queue>
#include <bitset>

using namespace mcts;

void Node::initialize(State const& state) {
  hash = NodeTable::key(state.hash);
}

void Node::update(double result) {
  double variance0 = variance(this);
  results(result);
  double variance1 = variance(this);
  variance_derivatives(variance1 - variance0);
}

double Node::selection_criterion(Node const* node) {
  if (sample_size(node) < 5)
    return 1e6;
  return mean(node) + sqrt(variance(node)) - sqrt(variance_derivative(node));
}

void Node::adjoin_parent(Node* parent) {
  std::lock_guard<std::mutex> lock(parents_mutex);
  parents.insert(parent);
}

double Node::rollout(State& state, boost::mt19937& generator) {
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
  return *winner == initial_player ? win_value : loss_value;
}

std::string Node::format_statistics() {
  return str(boost::format("%1% %2% %3% (%4% derivative) (%5% selection) (%6% parents)") % sample_size(this) % mean(this) % variance(this) % variance_derivative(this) % selection_criterion(this) % parents.size());
}

Node* NodeTable::get(Hash hash) {
  size_t key = NodeTable::key(hash);
  Node* node = &nodes.at(key);
  if (node->hash != key)
    return nullptr;
  return node;
}

Node* NodeTable::get_or_create(State const& state) {
  size_t key = NodeTable::key(state);
  Node* node = &nodes.at(key);
  if (node->hash != key)
    node->initialize(state);
  return node;
}

void Graph::sample(State state, boost::mt19937& generator) {
  Node* node = nodes.get_or_create(state);
  std::unordered_set<Hash> path;

  // selection
  while (Node::sample_size(node) > 0) {
    path.insert(node->hash);

    Node* child = select_child(node, state, generator);
    if (!child) {
      // no legal successors; loss
      backprop(node, loss_value);
      return;
    }
    if (path.count(child->hash) > 0) {
      // repeated state selected; draw by repetition
      backprop(node, draw_value);
      return;
    }
    node = child;
  }

  double result = node->rollout(state, generator);
  backprop(node, result);
}

// returns nullptr if no legal successor states
// NOTE: state will be modified to be the corresponding successor
Node* Graph::select_child(Node* node, State& state, boost::mt19937& generator) {
  std::vector<Move> moves;
  moves::moves(moves, state);

  // find a legal move with maximum selection criterion.
  boost::optional<Move> best_move;
  Node* best_child;
  double best_score;
  for (Move curr_move: moves) {
    Undo undo = state.make_move(curr_move);
    Node* curr_child = nodes.get_or_create(state);

    bool new_node = curr_child->parents.empty();
    curr_child->adjoin_parent(node);
    if (new_node)
      // select new nodes unconditionally
      return curr_child;

    // if legal
    if (!state.their_king_attacked()) {
      double curr_score = Node::selection_criterion(curr_child);
      if (!best_move || curr_score > best_score) {
        best_move  = curr_move;
        best_child = curr_child;
        best_score = curr_score;
      }
    }
    
    state.unmake_move(undo);
  }

  if (!best_move)
    return nullptr;
  
  state.make_move(*best_move);
  return best_child;
}

// update the node and all of its ancestors.  the graph is likely to be
// cyclic due to reversible moves.  we traverse it in a breadth-first
// manner so that the updates run along the shortest paths to each
// unique ancestor.
// NOTE: initial_result is from the perspective of the player who is to move in
// the state corresponding to node.  since the stored node values are from the
// perspective of the player who causes the node to be chosen, we have to invert
// it once.
void Graph::backprop(Node* node, double initial_result) {
  initial_result = invert_result(initial_result);

  std::unordered_set<Hash> encountered_nodes;
  auto encountered = [&](Node* node) {
    if (encountered_nodes.count(node->hash) > 0) {
      return true;
    } else {
      encountered_nodes.insert(node->hash);
      return false;
    }
  };

  std::queue<std::pair<Node*, double> > backlog;
  backlog.emplace(node, initial_result);
  
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  State known_win_state("rn4nr/p4N1p/8/1p4Qk/1Pp4P/8/PP1PPP1P/RNB1KBR1 b Q - 0 0");
#endif

  while (!backlog.empty()) {
    std::pair<Node*, double> pair = backlog.front();
    backlog.pop();
    
    Node* node = pair.first;
    double result = pair.second;

#ifdef MC_EXPENSIVE_RUNTIME_TESTS
    //if (node->hash == NodeTable::key(known_win_state.hash)) {
      // assertion commented out because hash collisions can cause it to fail.
      // uncomment it to get a backtrace when the relevant test case fails.
//      assert(result == win_value);
    //}
#endif
    node->update(result);

    double parent_result = invert_result(result);
    node->do_parents([&](Node* parent) {
        if (!encountered(parent))
          backlog.emplace(parent, parent_result);
      });

    assert(backlog.size() < 1e4);
  }
}

template <typename F>
boost::optional<Move> Graph::select_successor_by(State state, F f) {
  Node* node = nodes.get_or_create(state);
  boost::optional<Move> best_move;
  double best_score;
  node->do_successors(state, [&](State const& state, Move curr_move) {
      Node* curr_node = nodes.get_or_create(state);
      double curr_score = f(curr_node);
      if (!best_move || best_score < curr_score) {
        best_move  = curr_move;
        best_score = curr_score;
      }
    });
  return best_move;
}

void Graph::print_statistics(std::ostream& os, State state) {
  Node* node = nodes.get_or_create(state);
  os << node->format_statistics() << std::endl;
  node->do_successors(state, [&](State const& state, Move last_move) {
      Node* node = nodes.get_or_create(state);
      os << last_move << " " << node->format_statistics() << std::endl;
    });
}

boost::optional<Move> Graph::principal_move(State state) {
  // NOTE: we don't simply select the move with the best score. we also
  // want the subtree to be well explored.  the most-explored node should
  // have both of these properties.
  return select_successor_by(state, Node::sample_size);
}

void Graph::print_principal_variation(std::ostream& os, State state) {
  std::set<Hash> path;
  print_principal_variation(os, state, path);
}

void Graph::print_principal_variation(std::ostream& os, State state, std::set<Hash>& path) {
  boost::optional<Move> move = principal_move(state);
  if (!move)
    return;
  state.make_move(*move);
  Node* child = nodes.get_or_create(state);
  if (Node::sample_size(child) == 0 || path.count(child->hash) > 0)
    return;
  path.insert(child->hash);
  os << *move << " " << child->format_statistics() << std::endl;
  print_principal_variation(os, state, path);
}

// FIXME: handle cycles
void Graph::graphviz(std::ostream& os, Node* node, State state, boost::optional<Move> last_move = boost::none) {
  auto hash_as_id = [this](Hash hash) {
    return str(boost::format("\"%1$#8x\"") % hash);
  };
  os << hash_as_id(state.hash)
     << "[label=\"{"
     << (last_move ? notation::coordinate::format(*last_move) : "-")
     << " | "
     << node->format_statistics()
     << "}\"];"
     << std::endl;
  node->do_successors(state, [&](State state, Move last_move) {
      // only include existing nodes (to keep the graph small)
      Node* child = nodes.get(state.hash);
      if (!child)
        return;
      graphviz(os, node, state, last_move);
      os << hash_as_id(node->hash) << " -> " << hash_as_id(child->hash) << ";" << std::endl;
    });
}
