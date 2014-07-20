#pragma once

#include <mutex>
#include <cmath>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/serialization/vector.hpp>

#include "state.hpp"
#include "sorted_vector.hpp"

namespace mcts {
  namespace ac = boost::accumulators;

  const double
    loss_value = 0,
    draw_value = 0.5,
    win_value = 1;

  static inline double invert_result(double result) {
    return 1 - result;
  }

  class Node {
    size_t m_sample_size;
    // mean empirical result
    double m_mean;
    // estimated derivative of mean with respect to sample size
    double m_derivative;

  public:
    sorted_vector<Hash> parents;
    std::mutex parents_mutex;

    Hash hash;

    friend class boost::serialization::access;
    template<class Archive>
    inline void serialize(Archive& a, const unsigned int version) {
      a & m_sample_size;
      a & m_mean;
      a & m_derivative;
      a & hash;
      a & parents;
    }

    inline Node()
      : m_sample_size(0),
        m_mean(draw_value),
        m_derivative(0)
    {}

    // serialization requires a copy constructor.
    inline Node(Node const& that)
      : m_sample_size(that.m_sample_size),
        m_mean(that.m_mean),
        m_derivative(that.m_derivative),
        parents(that.parents),
        hash(that.hash)
    {}

    void initialize(State const& state);
    void adjoin_parent(Node* parent);
    double rollout(State& state, boost::mt19937& generator);
    inline void update(double result);

    inline double sample_size() const { return m_sample_size; }
    inline double mean() const { return m_mean; }
    inline double derivative() const { return m_derivative; }
    inline double selection_criterion() const {
      return m_mean + 10*m_derivative;
    }
    inline double selection_criterion(boost::mt19937& generator) const {
      double noise = standard_normal_distribution(generator);
      return selection_criterion() + m_derivative*noise;
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
      for (Hash parent: parents) {
        f(parent);
      }
    }

    std::string format_statistics();
  };

  class NodeTable {
    static const size_t HASH_KEY_LENGTH = 23;
    static const size_t HASH_KEY_CARDINALITY = 1 << HASH_KEY_LENGTH;
    static_assert(HASH_KEY_CARDINALITY > 0, "size_t too small to contain key cardinality");
    static const size_t HASH_KEY_MASK = HASH_KEY_CARDINALITY - 1;
    std::vector<Node> nodes;

  public:
    NodeTable()
      : nodes(HASH_KEY_CARDINALITY)
    {
    }

    static inline size_t key(const State& state) {
      return key(state.hash);
    }

    static inline size_t key(Hash hash) {
      return hash & HASH_KEY_MASK;
    }

    Node* get(Hash hash);
    Node* get_or_create(State const& state);

    friend class boost::serialization::access;
    template<class Archive>
    inline void serialize(Archive& a, const unsigned int version) {
      a & hashes::get_hashes();

      // if writing, store hash key length
      // if reading, check hash key length
      // need to put it into a temporary because HASH_KEY_LENGTH is const
      size_t hash_key_length = HASH_KEY_LENGTH;
      a & hash_key_length;
      if (hash_key_length != HASH_KEY_LENGTH)
        throw std::runtime_error("stored data uses different hash key length");

      a & nodes;
    }
  };

  class Graph {
    NodeTable nodes;

  public:
    void sample(State state, boost::mt19937& generator);
    Node* select_child(Node* node, State& state, boost::mt19937& generator);
    void backprop(Node* node, double initial_result);
    template <typename F> inline boost::optional<Move> select_successor_by(State state, F f);
    boost::optional<Move> principal_move(State state);
    void print_statistics(std::ostream& os, State state);
    void print_principal_variation(std::ostream& os, State state);
    void print_principal_variation(std::ostream& os, State state, sorted_vector<Hash>& path);
    void graphviz(std::ostream& os, Node* node, State state, boost::optional<Move> last_move);

    friend class boost::serialization::access;
    template<class Archive>
    inline void serialize(Archive& a, const unsigned int version) {
      a & nodes;
    }
  };
}
