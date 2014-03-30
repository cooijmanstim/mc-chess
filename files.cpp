#include "files.hpp"
#include "direction.hpp"

BoardPartition files::partition = [](){
  return BoardPartition({"a", "b", "c", "d", "e", "f", "g", "h"},
                        [](size_t index) {
                          return Bitboard(0x0101010101010101) << index*directions::east;
                        });
}();
