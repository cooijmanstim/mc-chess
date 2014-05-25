#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "state.hpp"

namespace mcts {
  class Node;

  typedef boost::shared_ptr<Node> FarNode;
  typedef double Result;

  class Node : public boost::enable_shared_from_this<Node> {
    FarNode parent;
    Hash state_hash;
    boost::optional<Move> last_move;
    Color last_player;
    std::vector<Move> unexplored_moves;
    std::vector<FarNode> children;
    Result total_result;
    size_t nvisits;

    Node();

  public:
    static FarNode create(FarNode parent, boost::optional<Move> move, const State& state);
    FarNode add_child(Move move, const State& state);
    void sample(State state, boost::mt19937& generator);
    FarNode select(State& state);
    FarNode expand(State& state, boost::mt19937& generator);
    Result rollout(State& state, boost::mt19937& generator);
    void backprop(Result result);
    void update(Result result);
    static Result invert_result(Result result);
    static double uct_score(FarNode child);
    static double winrate(FarNode child);
    static double most_visited(FarNode child);
    FarNode select_by(std::function<double(FarNode)>);
    Move best_move();
    void print_evaluations();
  };
}
