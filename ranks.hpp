#pragma once

#include "boardpartition.hpp"

namespace ranks {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  const BoardPartition::Part _1 = partition[0],
                             _2 = partition[1],
                             _3 = partition[2],
                             _4 = partition[3],
                             _5 = partition[4],
                             _6 = partition[5],
                             _7 = partition[6],
                             _8 = partition[7];
}
