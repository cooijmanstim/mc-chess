#include "ranks.hpp"
#include "direction.hpp"

BoardPartition ranks::partition = [](){
  return BoardPartition({"1", "2", "3", "4", "5", "6", "7", "8"},
                        [](size_t index) {
                          return Bitboard(0x00000000000000FF) << index*directions::north;
                        });
}();
