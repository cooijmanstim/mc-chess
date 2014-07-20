#include "mcts.hpp"
#include "notation.hpp"

#include <stack>

#include <boost/range/algorithm/random_shuffle.hpp>

using namespace mcts;

void Node::initialize(State const& state) {
  hash = NodeTable::key(state.hash);
}

void Node::update(double result) {
  m_sample_size++;
  double mean0 = m_mean;
  double delta = result - mean0;
  m_mean += delta/m_sample_size;
  m_derivative = 0.9*m_derivative + 0.1*(m_mean - mean0);
}

void Node::adjoin_parent(Node* parent) {
  std::lock_guard<std::mutex> lock(parents_mutex);
  parents.insert(parent->hash);
}

std::string Node::format_statistics() {
  return str(boost::format("%1% %2% (d %3%) %4% (%5% parents)")
             % sample_size()
             % mean()
             % derivative()
             % selection_criterion()
             % parents.size());
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
  sorted_vector<Hash> path;

  double result;

  // selection
  while (true) {
    path.insert(node->hash);

    Node* child = select_child(node, state, generator);

    if (!child) {
      // no legal successors; game over
      boost::optional<Color> winner = state.winner();
      result = !winner
        ? draw_value
        : (*winner == state.us
           ? win_value
           : loss_value);
      break;
    }

    if (state.drawn_by_50() || path.contains(child->hash)) {
      // draw per 50-move rule or by choosing repetition
      result = draw_value;
      break;
    }

    node = child;
  }

  backprop(node, result);
}

// returns nullptr if no legal successor states
// NOTE: state will be modified to be the corresponding successor
Node* Graph::select_child(Node* node, State& state, boost::mt19937& generator) {
  if (node->sample_size() < 30) {
    // don't waste time with unreliable statistics
    boost::optional<Move> move = moves::make_random_legal_move(state, generator);
    if (!move)
      return nullptr;
    Node* child = nodes.get_or_create(state);
    child->adjoin_parent(node);
    return child;
  } else {
    // use statistics to make selection
    std::vector<Move> moves;
    moves::moves(moves, state);

    // find a legal move with maximum selection criterion.
    boost::optional<Move> best_move;
    Node* best_child;
    double best_score;
    
    for (Move curr_move: moves) {
      Undo undo = state.make_move(curr_move);
      
      // if legal
      if (!state.their_king_attacked()) {
        Node* curr_child = nodes.get_or_create(state);
        
        curr_child->adjoin_parent(node);
        if (curr_child->sample_size() < 10)
          // select new nodes unconditionally
          return curr_child;
        
        double curr_score = curr_child->selection_criterion(generator);
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
  auto encountered = [&](Hash hash) {
    if (encountered_nodes.count(hash) > 0) {
      return true;
    } else {
      encountered_nodes.insert(hash);
      return false;
    }
  };

  std::stack<std::pair<Node*, double> > backlog;
  backlog.emplace(node, initial_result);
  
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  State known_win_state("rn4nr/p4N1p/8/1p4Qk/1Pp4P/8/PP1PPP1P/RNB1KBR1 b Q - 0 0");
#endif

  while (!backlog.empty()) {
    std::pair<Node*, double> pair = backlog.top();
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
    node->do_parents([&](Hash parent) {
        if (!encountered(parent))
          backlog.emplace(nodes.get(parent), parent_result);
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
  return select_successor_by(state, [](Node const* node) {
      return node->mean();
    });
}

void Graph::print_principal_variation(std::ostream& os, State state) {
  sorted_vector<Hash> path;
  print_principal_variation(os, state, path);
}

void Graph::print_principal_variation(std::ostream& os, State state, sorted_vector<Hash>& path) {
  boost::optional<Move> move = principal_move(state);
  if (!move)
    return;
  state.make_move(*move);
  Node* child = nodes.get_or_create(state);
  if (child->sample_size() == 0 || path.contains(child->hash))
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
