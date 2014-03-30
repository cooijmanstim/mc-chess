#pragma once

#include "boardpartition.hpp"

namespace antidiagonals {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  const BoardPartition::Part a8a8 = partition[0],
                             a7b8 = partition[1],
                             a6c8 = partition[2],
                             a5d8 = partition[3],
                             a4e8 = partition[4],
                             a3f8 = partition[5],
                             a2g8 = partition[6],
                             a1h8 = partition[7],
                             b1h7 = partition[8],
                             c1h6 = partition[9],
                             d1h5 = partition[10],
                             e1h4 = partition[11],
                             f1h3 = partition[12],
                             g1h2 = partition[13],
                             h1h1 = partition[14];
}
