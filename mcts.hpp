#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "state.hpp"

namespace mcts {
  class Node;

  typedef double Result;

  class Node {
    Node* parent;
    Node* children;
    size_t child_count;

    Hash state_hash;
    boost::optional<Move> last_move;

    Result total_result;
    size_t visit_count;

  public:
    Node(Node* parent, boost::optional<Move> move, const State& state);
    ~Node();
    Result invert_result(Result result);
    Node* find_node(Hash state_hash, unsigned depth);
    void sample(State state, boost::mt19937& generator);
    Node* select(State& state);
    Node* expand(State& state, boost::mt19937& generator);
    Result rollout(State& state, boost::mt19937& generator);
    void backprop(Result result);
    void update(Result result);
    static double winrate(Node* child);
    static double uct_score(Node* child);
    static double most_visited(Node* child);
    Node* select_by(std::function<double(Node*)> key);
    void do_children(std::function<void(Node*)> f);
    Move best_move();
    void print_evaluations();
  };
}
