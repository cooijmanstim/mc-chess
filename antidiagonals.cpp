#include "antidiagonals.hpp"
#include "direction.hpp"

BoardPartition antidiagonals::partition = [](){
  return BoardPartition({ "a8a8", "a7b8", "a6c8", "a5d8", "a4e8", "a3f8", "a2g8", "a1h8", "b1h7", "c1h6", "d1h5", "e1h4", "f1h3", "g1h2", "h1h1" },
                        [](size_t index) {
                          Bitboard maindiagonal = Bitboard(0x8040201008040201);
                          int shiftcount = (7 - index)*directions::north;
                          return index < 7 ? maindiagonal << shiftcount : maindiagonal >> -shiftcount;
                        });
}();
