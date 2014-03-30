#include "diagonals.hpp"
#include "direction.hpp"

BoardPartition diagonals::partition = [](){
  return BoardPartition({ "h8h8", "g8h7", "f8h6", "e8h5", "d8h4", "c8h3", "b8h2", "a8h1", "a7g1", "a6f1", "a5e1", "a4d1", "a3c1", "a2b1", "a1a1" },
                        [](size_t index) {
                          Bitboard maindiagonal = Bitboard(0x0102040810204080);
                          int shiftcount = (7 - index)*directions::north;
                          return index < 7 ? maindiagonal >> shiftcount : maindiagonal << -shiftcount;
                        });
}();
