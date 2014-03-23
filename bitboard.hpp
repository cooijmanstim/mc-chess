#pragma once

#include <cstdint>
#include <array>
#include <functional>

#include "direction.hpp"

typedef uint64_t Bitboard;

namespace squares {
  const size_t cardinality = 64;

#define _(i) Bitboard(1)<<(i)
  const Bitboard a1 = _( 0), b1 = _( 1), c1 = _( 2), d1 = _( 3), e1 = _( 4), f1 = _( 5), g1 = _( 6), h1 = _( 7),
                 a2 = _( 8), b2 = _( 9), c2 = _(10), d2 = _(11), e2 = _(12), f2 = _(13), g2 = _(14), h2 = _(15),
                 a3 = _(16), b3 = _(17), c3 = _(18), d3 = _(19), e3 = _(20), f3 = _(21), g3 = _(22), h3 = _(23),
                 a4 = _(24), b4 = _(25), c4 = _(26), d4 = _(27), e4 = _(28), f4 = _(29), g4 = _(30), h4 = _(31),
                 a5 = _(32), b5 = _(33), c5 = _(34), d5 = _(35), e5 = _(36), f5 = _(37), g5 = _(38), h5 = _(39),
                 a6 = _(40), b6 = _(41), c6 = _(42), d6 = _(43), e6 = _(44), f6 = _(45), g6 = _(46), h6 = _(47),
                 a7 = _(48), b7 = _(49), c7 = _(50), d7 = _(51), e7 = _(52), f7 = _(53), g7 = _(54), h7 = _(55),
                 a8 = _(56), b8 = _(57), c8 = _(58), d8 = _(59), e8 = _(60), f8 = _(61), g8 = _(62), h8 = _(63);

  typedef size_t Index;

  const std::array<Bitboard, cardinality> byIndex = [](){
    std::array<Bitboard, cardinality> byIndex;
    for (size_t i = 0; i < cardinality; i++)
      byIndex[i] = _(i);
    return byIndex;
  }();
#undef _
  
  std::string name_from_index(squares::Index i);
  Index index_from_bitboard(Bitboard b);
  std::string name_from_bitboard(Bitboard b);
}

namespace bitboard {
  squares::Index scan_forward(Bitboard b);
  squares::Index scan_forward_with_reset(Bitboard& b);
  bool is_empty(Bitboard b);
  void for_each_member(Bitboard b, std::function<void(squares::Index)> f);
}

namespace files {
  const size_t cardinality = 8;

  using namespace directions;
#define MAINFILE Bitboard(0x0101010101010101)
  const Bitboard a = MAINFILE << 0*east,
                 b = MAINFILE << 1*east,
                 c = MAINFILE << 2*east,
                 d = MAINFILE << 3*east,
                 e = MAINFILE << 4*east,
                 f = MAINFILE << 5*east,
                 g = MAINFILE << 6*east,
                 h = MAINFILE << 7*east;
#undef MAINFILE

  const std::array<Bitboard, squares::cardinality> bySquareIndex = [](){
    std::array<Bitboard, squares::cardinality> bySquareIndex;
    std::array<Bitboard, cardinality> files = {a,b,c,d,e,f,g,h};
    for (size_t i = 0; i < squares::cardinality; i++)
      bySquareIndex[i] = files[i % cardinality];
    return bySquareIndex;
  }();
}

namespace ranks {
  const size_t cardinality = 8;

  using namespace directions;
#define MAINRANK Bitboard(0x00000000000000FF)
  const Bitboard _1 = MAINRANK << 0*north,
                 _2 = MAINRANK << 1*north,
                 _3 = MAINRANK << 2*north,
                 _4 = MAINRANK << 3*north,
                 _5 = MAINRANK << 4*north,
                 _6 = MAINRANK << 5*north,
                 _7 = MAINRANK << 6*north,
                 _8 = MAINRANK << 7*north;
#undef MAINRANK

  const std::array<Bitboard, squares::cardinality> bySquareIndex = [](){
    std::array<Bitboard, squares::cardinality> bySquareIndex;
    std::array<Bitboard, cardinality> ranks = {_1,_2,_3,_4,_5,_6,_7,_8};
    for (size_t i = 0; i < squares::cardinality; i++)
      bySquareIndex[i] = ranks[i / files::cardinality];
    return bySquareIndex;
  }();
}

namespace diagonals {
  const size_t cardinality = 18;

  using namespace directions;
#define MAINDIAG Bitboard(0x8040201008040201)
  const Bitboard _8 = MAINDIAG << 7*north,
                 _7 = MAINDIAG << 6*north,
                 _6 = MAINDIAG << 5*north,
                 _5 = MAINDIAG << 4*north,
                 _4 = MAINDIAG << 3*north,
                 _3 = MAINDIAG << 2*north,
                 _2 = MAINDIAG << 1*north,
                 _1 = MAINDIAG << 0*north,
                 a  = MAINDIAG >> 0*north,
                 b  = MAINDIAG >> 1*north,
                 c  = MAINDIAG >> 2*north,
                 d  = MAINDIAG >> 3*north,
                 e  = MAINDIAG >> 4*north,
                 f  = MAINDIAG >> 5*north,
                 g  = MAINDIAG >> 6*north,
                 h  = MAINDIAG >> 7*north;
#undef MAINDIAG

  const std::array<Bitboard, squares::cardinality> bySquareIndex = [](){
    std::array<Bitboard, squares::cardinality> bySquareIndex;
    std::array<Bitboard, cardinality> diagonals = {_8,_7,_6,_5,_4,_3,_2,_1,a,b,c,d,e,f,g,h};
    for (size_t i = 0; i < cardinality; i++) {
      bitboard::for_each_member(diagonals[i], [&bySquareIndex, &diagonals, i](squares::Index j) {
          bySquareIndex[j] = diagonals[i];
        });
    }
    return bySquareIndex;
  }();
}

namespace antidiagonals {
  const size_t cardinality = 18;

  using namespace directions;
#define MAINDIAG Bitboard(0x0102040810204080)
  const Bitboard h  = MAINDIAG << 7*north,
                 g  = MAINDIAG << 6*north,
                 f  = MAINDIAG << 5*north,
                 e  = MAINDIAG << 4*north,
                 d  = MAINDIAG << 3*north,
                 c  = MAINDIAG << 2*north,
                 b  = MAINDIAG << 1*north,
                 a  = MAINDIAG << 0*north,
                 _8 = MAINDIAG >> 0*north,
                 _7 = MAINDIAG >> 1*north,
                 _6 = MAINDIAG >> 2*north,
                 _5 = MAINDIAG >> 3*north,
                 _4 = MAINDIAG >> 4*north,
                 _3 = MAINDIAG >> 5*north,
                 _2 = MAINDIAG >> 6*north,
                 _1 = MAINDIAG >> 7*north;
#undef MAINDIAG

  const std::array<Bitboard, squares::cardinality> bySquareIndex = [](){
    std::array<Bitboard, squares::cardinality> bySquareIndex;
    std::array<Bitboard, cardinality> diagonals = {_8,_7,_6,_5,_4,_3,_2,_1,a,b,c,d,e,f,g,h};
    for (size_t i = 0; i < cardinality; i++) {
      bitboard::for_each_member(diagonals[i], [&bySquareIndex, &diagonals, i](squares::Index j) {
          bySquareIndex[j] = diagonals[i];
        });
    }
    return bySquareIndex;
  }();
}

