#include "mcts.hpp"
#include "notation.hpp"

#include <queue>

using namespace mcts;

const double
  loss_value = 0,
  draw_value = 0.5,
  win_value = 1;

double Node::rollout(State& state, boost::mt19937& generator) {
  Color initial_player = state.us;
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  State initial_state(state);
  std::vector<Move> move_history; // for debugging
#endif
  while (true) {
    if (state.drawn_by_50())
      return draw_value;
    //boost::optional<Move> move = moves::random_move(state, generator);
    boost::optional<Move> move = moves::make_random_legal_move(state, generator);
    if (!move)
      break;
    //state.make_move(*move);
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
  return str(boost::format("%1% %2% %3% %4%") % sample_size(this) % mean(this) % variance(this) % selection_criterion(this));
}

Node* Graph::get_node(Hash hash) {
  std::lock_guard<std::mutex> lock(nodes_mutex);
  if (nodes.count(hash) == 0)
    return nullptr;
  return &nodes.at(hash);
}

Node* Graph::get_or_create_node(State const& state) {
  std::lock_guard<std::mutex> lock(nodes_mutex);
  Node *node = &nodes[state.hash];
  if (node->hash != state.hash)
    node->initialize(state);
  return node;
}

void Graph::sample(State state, boost::mt19937& generator) {
  Node* node = get_or_create_node(state);
  std::set<Hash> path;

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

  Hash parent_hash = state.hash;

  // find a legal move with maximum selection criterion.
  boost::optional<Move> best_move;
  Node* best_child;
  double best_score;
  for (Move curr_move: moves) {
    Undo undo = state.make_move(curr_move);
    Node* curr_child = get_or_create_node(state);

    {
      std::lock_guard<std::mutex> lock(curr_child->parents_mutex);
      // new node; select it unconditionally
      bool new_node = curr_child->parents.empty();
      curr_child->parents.insert(parent_hash);
      if (new_node)
        return curr_child;
    }

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
  initial_result = Node::invert_result(initial_result);

  std::set<Hash> done;
  std::queue<std::pair<Hash, double> > backlog;
  backlog.emplace(node->hash, initial_result);
  
  while (!backlog.empty()) {
    std::pair<Hash, double> pair = backlog.front();
    backlog.pop();
    
    Node* node = get_node(pair.first);

#ifdef MC_EXPENSIVE_RUNTIME_TESTS
    State state("rn4nr/p4N1p/8/1p4Qk/1Pp4P/8/PP1PPP1P/RNB1KBR1 b Q - 0 0");
    if (state.hash == pair.first)
      assert(pair.second == win_value);
#endif
    node->update(pair.second);

    done.insert(pair.first);
    
    double parent_result = Node::invert_result(pair.second);
    {
      node->do_parents([&](Hash parent_hash) {
          if (done.count(parent_hash) == 0)
            backlog.emplace(parent_hash, parent_result);
        });
    }
  }
}

template <typename F>
boost::optional<Move> Graph::select_successor_by(State state, F f) {
  Node* node = get_or_create_node(state);
  boost::optional<Move> best_move;
  double best_score;
  node->do_successors(state, [&](State const& state, Move curr_move) {
      Node* curr_node = get_or_create_node(state);
      double curr_score = f(curr_node);
      if (!best_move || best_score < curr_score) {
        best_move  = curr_move;
        best_score = curr_score;
      }
    });
  return best_move;
}

void Graph::print_statistics(std::ostream& os, State state) {
  Node* node = get_or_create_node(state);
  os << node->format_statistics() << std::endl;
  node->do_successors(state, [&](State const& state, Move last_move) {
      Node* node = get_or_create_node(state);
      os << last_move << " " << node->format_statistics() << std::endl;
    });
}

boost::optional<Move> Graph::principal_move(State state) {
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
  Node* child = get_or_create_node(state);
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
      Node* child = get_node(state.hash);
      if (!child)
        return;
      graphviz(os, node, state, last_move);
      os << hash_as_id(node->hash) << " -> " << hash_as_id(child->hash) << ";" << std::endl;
    });
}
