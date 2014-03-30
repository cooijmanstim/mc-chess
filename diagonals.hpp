#pragma once

#include "boardpartition.hpp"

namespace diagonals {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  const BoardPartition::Part h8h8 = partition[0],
                             g8h7 = partition[1],
                             f8h6 = partition[2],
                             e8h5 = partition[3],
                             d8h4 = partition[4],
                             c8h3 = partition[5],
                             b8h2 = partition[6],
                             a8h1 = partition[7],
                             a7g1 = partition[8],
                             a6f1 = partition[9],
                             a5e1 = partition[10],
                             a4d1 = partition[11],
                             a3c1 = partition[12],
                             a2b1 = partition[13],
                             a1a1 = partition[14];
}

