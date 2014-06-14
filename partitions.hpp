#pragma once

#include <map>

#include <boost/random.hpp>

#include "bitboard.hpp"
#include "direction.hpp"

namespace squares {
#define PARTITION_NAMESPACE squares
#define PARTITION_CARDINALITY 64
#define PARTITION_KEYWORDS \
  _(a1) _(b1) _(c1) _(d1) _(e1) _(f1) _(g1) _(h1) \
  _(a2) _(b2) _(c2) _(d2) _(e2) _(f2) _(g2) _(h2) \
  _(a3) _(b3) _(c3) _(d3) _(e3) _(f3) _(g3) _(h3) \
  _(a4) _(b4) _(c4) _(d4) _(e4) _(f4) _(g4) _(h4) \
  _(a5) _(b5) _(c5) _(d5) _(e5) _(f5) _(g5) _(h5) \
  _(a6) _(b6) _(c6) _(d6) _(e6) _(f6) _(g6) _(h6) \
  _(a7) _(b7) _(c7) _(d7) _(e7) _(f7) _(g7) _(h7) \
  _(a8) _(b8) _(c8) _(d8) _(e8) _(f8) _(g8) _(h8)
#define PARTITION_BITBOARD(i) (Bitboard(1) << (i));

#include "partition_template.hpp"

  inline Index index(Bitboard b) {
    assert(bitboard::cardinality(b) == 1);
    return static_cast<Index>(bitboard::scan_forward(b));
  }

  inline Index random_index(Bitboard b, boost::mt19937& generator) {
    return static_cast<Index>(bitboard::random_index(b, generator));
  }

  inline Index flip_vertically(Index index) {
    return static_cast<Index>(index ^ 56);
  }

  template <typename F>
  inline void for_each(Bitboard b, F f) {
    bitboard::for_each_member(b, [&](size_t index) {
        f(static_cast<squares::Index>(index));
      });
  }

  template <typename F>
  inline bool any(Bitboard b, F f) {
    while (!bitboard::is_empty(b)) {
      squares::Index index = static_cast<squares::Index>(bitboard::scan_forward_with_reset(b));
      if (f(index))
        return true;
    }
    return false;
  }
}

// the partitions below all need to have this procedure to look up a part by square index
#define BY_SQUARE(spacename) \
  inline Index by_square(squares::Index si) { \
    static auto by_square = [](){ \
      std::array<Index, squares::cardinality> by_square; \
      for (squares::Index si: squares::indices) { \
        for (Index i: indices) { \
          if (squares::bitboard(si) & bitboard(i)) \
            by_square[si] = i; \
        } \
      } \
      return by_square; \
    }(); \
    return by_square[si]; \
  } \
  \
  namespace bitboards { \
    inline Bitboard by_square(squares::Index si) { \
      return bitboard(spacename::by_square(si)); \
    } \
  }

namespace files {
#define PARTITION_NAMESPACE files
#define PARTITION_CARDINALITY 8
#define PARTITION_KEYWORDS _(a) _(b) _(c) _(d) _(e) _(f) _(g) _(h)
#define PARTITION_BITBOARD(i) (Bitboard(0x0101010101010101) << i*directions::horizontal)
#include "partition_template.hpp"
BY_SQUARE(files)
}

namespace ranks {
#define PARTITION_NAMESPACE ranks
#define PARTITION_CARDINALITY 8
#define PARTITION_KEYWORDS _(_1) _(_2) _(_3) _(_4) _(_5) _(_6) _(_7) _(_8)
#define PARTITION_BITBOARD(i) (Bitboard(0x00000000000000FF) << i*directions::vertical)
#include "partition_template.hpp"
BY_SQUARE(ranks)
}

namespace diagonals {
#define PARTITION_NAMESPACE diagonals
#define PARTITION_CARDINALITY 15
#define PARTITION_KEYWORDS _(h8h8) _(g8h7) _(f8h6) _(e8h5) _(d8h4) _(c8h3) _(b8h2) _(a8h1) _(a7g1) _(a6f1) _(a5e1) _(a4d1) _(a3c1) _(a2b1) _(a1a1)
#define PARTITION_BITBOARD(i) ([&i](){ \
  Bitboard maindiagonal = Bitboard(0x0102040810204080); \
  int shiftcount = (7 - i)*directions::vertical; \
  return shiftcount >= 0 ? maindiagonal >> shiftcount : maindiagonal << -shiftcount; \
})()
#include "partition_template.hpp"
BY_SQUARE(diagonals)
}

namespace giadonals {
#define PARTITION_NAMESPACE giadonals
#define PARTITION_CARDINALITY 15
#define PARTITION_KEYWORDS _(a8a8) _(a7b8) _(a6c8) _(a5d8) _(a4e8) _(a3f8) _(a2g8) _(a1h8) _(b1h7) _(c1h6) _(d1h5) _(e1h4) _(f1h3) _(g1h2) _(h1h1)
#define PARTITION_BITBOARD(i) ([&i](){ \
  Bitboard maingiadonal = Bitboard(0x8040201008040201); \
  int shiftcount = (7 - i)*directions::vertical; \
  return shiftcount >= 0 ? maingiadonal << shiftcount : maingiadonal >> -shiftcount; \
})()
#include "partition_template.hpp"
BY_SQUARE(giadonals)
}
