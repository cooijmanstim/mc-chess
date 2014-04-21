#pragma once

#include "trivialboardpartition.hpp"

namespace squares {
  extern TrivialBoardPartition partition;
  typedef TrivialBoardPartition::Index Index;

  extern const TrivialBoardPartition::Part a1, b1, c1, d1, e1, f1, g1, h1,
                                           a2, b2, c2, d2, e2, f2, g2, h2,
                                           a3, b3, c3, d3, e3, f3, g3, h3,
                                           a4, b4, c4, d4, e4, f4, g4, h4,
                                           a5, b5, c5, d5, e5, f5, g5, h5,
                                           a6, b6, c6, d6, e6, f6, g6, h6,
                                           a7, b7, c7, d7, e7, f7, g7, h7,
                                           a8, b8, c8, d8, e8, f8, g8, h8;
  const size_t cardinality = 64;

  Bitboard bitboard_from_index(Index i);
  Index index_from_bitboard(Bitboard b);
  TrivialBoardPartition::Part from_bitboard(Bitboard b);
}

#include "boardpartition.hpp"

namespace files {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  extern const BoardPartition::Part a, b, c, d, e, f, g, h;
  const size_t cardinality = 8;
}

namespace ranks {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  extern const BoardPartition::Part _1, _2, _3, _4, _5, _6, _7, _8;
  const size_t cardinality = 8;
}

namespace diagonals {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  extern const BoardPartition::Part h8h8, g8h7, f8h6, e8h5, d8h4, c8h3, b8h2, a8h1, a7g1, a6f1, a5e1, a4d1, a3c1, a2b1, a1a1;
  const size_t cardinality = 15;
}

namespace giadonals {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  extern const BoardPartition::Part a8a8, a7b8, a6c8, a5d8, a4e8, a3f8, a2g8, a1h8, b1h7, c1h6, d1h5, e1h4, f1h3, g1h2, h1h1;
  const size_t cardinality = 15;
}

typedef TrivialBoardPartition::Part Square;
typedef BoardPartition::Part File;
typedef BoardPartition::Part Rank;
typedef BoardPartition::Part Diagonal;
typedef BoardPartition::Part Giadonal;
