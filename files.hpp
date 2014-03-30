#pragma once

#include "boardpartition.hpp"

namespace files {
  extern BoardPartition partition;
  typedef BoardPartition::Index Index;
  const BoardPartition::Part a = partition[0],
                             b = partition[1],
                             c = partition[2],
                             d = partition[3],
                             e = partition[4],
                             f = partition[5],
                             g = partition[6],
                             h = partition[7];
}
