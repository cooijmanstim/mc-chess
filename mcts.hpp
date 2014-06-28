#pragma once

#include <mutex>
#include <cmath>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "state.hpp"

namespace mcts {
  namespace ac = boost::accumulators;

  class Node : boost::noncopyable {
  public:
    std::set<Hash> parents;
    std::mutex parents_mutex;

    Hash hash;

    ac::accumulator_set<double, ac::stats<ac::tag::count, ac::tag::mean, ac::tag::variance> > statistics;

    inline void initialize(State state) {
      hash = state.hash;
    }

    double rollout(State& state, boost::mt19937& generator);

    static inline double invert_result(double result) {
      return 1 - result;
    }

    inline void update(double result) {
      statistics(result);
    }

    static inline size_t sample_size (Node const* node) { return ac::count(node->statistics); }
    static inline double mean (Node const* node) { return ac::mean(node->statistics); }
    static inline double variance (Node const* node) { return ac::variance(node->statistics); }
    static inline double selection_criterion (Node const* node) {
      // a la Iolis & Bontempi, "Comparison of Selection Strategies in
      // Monte-Carlo Tree Search for Computer Poker".  I don't distinguish
      // between internal nodes and leaf nodes because finding the children
      // of a node involves move generation and node lookups.
      if (sample_size(node) < 30)
        return 1e6;
      return mean(node) + sqrt(variance(node)) / log(sample_size(node));
    }

    template <typename F>
    inline void do_successors(State state, F f) {
      std::vector<Move> moves;
      moves::legal_moves(moves, state);
      for (Move move: moves) {
        Undo undo = state.make_move(move);
        f(state, move);
        state.unmake_move(undo);
      }
    }

    template <typename F>
    inline void do_parents(F f) {
      std::lock_guard<std::mutex> lock(parents_mutex);
      for (Hash parent_hash: parents) {
        f(parent_hash);
      }
    }

    std::string format_statistics();
  };

  class Graph {
    // TODO: having to lock on nodes all the time is really going to be a
    // performance drain.  use a custom hash table after all.
    std::map<Hash, Node> nodes;
    std::mutex nodes_mutex;

  public:
    Node* get_node(Hash hash);
    Node* get_or_create_node(State const& state);
    void sample(State state, boost::mt19937& generator);
    Node* select_child(Node* node, State& state, boost::mt19937& generator);
    void backprop(Node* node, double initial_result);
    template <typename F> inline boost::optional<Move> select_successor_by(State state, F f);
    boost::optional<Move> principal_move(State state);
    void print_statistics(std::ostream& os, State state);
    void print_principal_variation(std::ostream& os, State state);
    void print_principal_variation(std::ostream& os, State state, std::set<Hash>& path);
    void graphviz(std::ostream& os, Node* node, State state, boost::optional<Move> last_move);
  };
}
