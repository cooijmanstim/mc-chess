#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "state.hpp"

namespace mcts {
  class Node;

  typedef boost::shared_ptr<Node> FarNode;

  class Node : public boost::enable_shared_from_this<Node> {
    FarNode parent;
    Hash state_hash;
    boost::optional<Move> last_move;
    Color last_player;
    std::vector<Move> unexplored_moves;
    std::vector<FarNode> children;
    size_t total_result;
    size_t nvisits;

    Node();

  public:
    static FarNode create(FarNode parent, boost::optional<Move> move, const State& state);
    FarNode add_child(Move move, const State& state);
    void sample(State state, boost::mt19937& generator);
    FarNode select(State& state);
    FarNode expand(State& state, boost::mt19937& generator);
    int rollout(State& state, boost::mt19937& generator);
    void backprop(int result);
    void update(int result);
    static float uct_score(FarNode child);
    static float most_visited(FarNode child);
    FarNode select_by(std::function<float(FarNode)>);
    Move best_move();
  };
}
