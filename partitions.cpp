#include "partitions.hpp"
#include "direction.hpp"

namespace squares {
  TrivialBoardPartition partition = [](){
    return TrivialBoardPartition({ "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
                                   "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                                   "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                                   "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                                   "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                                   "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                                   "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                                   "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
                                  },
                                  [](size_t index) {
                                    return bitboard_from_index(index);
                                  });
  }();

#define _(i) partition[i]
  const TrivialBoardPartition::Part a1 = _( 0), b1 = _( 1), c1 = _( 2), d1 = _( 3), e1 = _( 4), f1 = _( 5), g1 = _( 6), h1 = _( 7),
                                    a2 = _( 8), b2 = _( 9), c2 = _(10), d2 = _(11), e2 = _(12), f2 = _(13), g2 = _(14), h2 = _(15),
                                    a3 = _(16), b3 = _(17), c3 = _(18), d3 = _(19), e3 = _(20), f3 = _(21), g3 = _(22), h3 = _(23),
                                    a4 = _(24), b4 = _(25), c4 = _(26), d4 = _(27), e4 = _(28), f4 = _(29), g4 = _(30), h4 = _(31),
                                    a5 = _(32), b5 = _(33), c5 = _(34), d5 = _(35), e5 = _(36), f5 = _(37), g5 = _(38), h5 = _(39),
                                    a6 = _(40), b6 = _(41), c6 = _(42), d6 = _(43), e6 = _(44), f6 = _(45), g6 = _(46), h6 = _(47),
                                    a7 = _(48), b7 = _(49), c7 = _(50), d7 = _(51), e7 = _(52), f7 = _(53), g7 = _(54), h7 = _(55),
                                    a8 = _(56), b8 = _(57), c8 = _(58), d8 = _(59), e8 = _(60), f8 = _(61), g8 = _(62), h8 = _(63);
#undef _

  Bitboard bitboard_from_index(Index i) {
    return Bitboard(1) << i;
  }

  Index index_from_bitboard(Bitboard b) {
    // assumption that exactly one bit is set
    return bitboard::scan_forward(b);
  }
  
  TrivialBoardPartition::Part from_bitboard(Bitboard b) {
    return partition[index_from_bitboard(b)];
  }
}

namespace files {
  BoardPartition partition = [](){
    return BoardPartition({"a", "b", "c", "d", "e", "f", "g", "h"},
                          [](size_t index) {
                            return Bitboard(0x0101010101010101) << index*directions::east;
                          });
  }();

  const BoardPartition::Part a = partition[0],
                             b = partition[1],
                             c = partition[2],
                             d = partition[3],
                             e = partition[4],
                             f = partition[5],
                             g = partition[6],
                             h = partition[7];
}

namespace ranks {
  BoardPartition partition = [](){
    return BoardPartition({"1", "2", "3", "4", "5", "6", "7", "8"},
                          [](size_t index) {
                            return Bitboard(0x00000000000000FF) << index*directions::north;
                          });
  }();

  const BoardPartition::Part _1 = partition[0],
                             _2 = partition[1],
                             _3 = partition[2],
                             _4 = partition[3],
                             _5 = partition[4],
                             _6 = partition[5],
                             _7 = partition[6],
                             _8 = partition[7];
}

namespace diagonals {
  BoardPartition partition = [](){
    return BoardPartition({ "h8h8", "g8h7", "f8h6", "e8h5", "d8h4", "c8h3", "b8h2", "a8h1", "a7g1", "a6f1", "a5e1", "a4d1", "a3c1", "a2b1", "a1a1" },
                          [](size_t index) {
                            Bitboard maindiagonal = Bitboard(0x0102040810204080);
                            int shiftcount = (7 - index)*directions::north;
                            return index < 7 ? maindiagonal >> shiftcount : maindiagonal << -shiftcount;
                          });
  }();

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

namespace giadonals {
  BoardPartition partition = [](){
    return BoardPartition({ "a8a8", "a7b8", "a6c8", "a5d8", "a4e8", "a3f8", "a2g8", "a1h8", "b1h7", "c1h6", "d1h5", "e1h4", "f1h3", "g1h2", "h1h1" },
                          [](size_t index) {
                            Bitboard maingiadonal = Bitboard(0x8040201008040201);
                            int shiftcount = (7 - index)*directions::north;
                            return index < 7 ? maingiadonal << shiftcount : maingiadonal >> -shiftcount;
                          });
  }();

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
