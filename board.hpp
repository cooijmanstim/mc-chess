#pragma once

#include "util.hpp"
#include "bitboard.hpp"
#include "colors.hpp"
#include "pieces.hpp"

typedef array2d<Bitboard, colors::cardinality, pieces::cardinality> Board;
typedef std::array<Bitboard, colors::cardinality> Occupancy;
typedef std::array<Bitboard, pieces::cardinality> Halfboard;

namespace board {
  void flatten(const Board& b, Occupancy& o);
  void flatten(const Board& b, Halfboard& h);
  void flatten(const Occupancy& o, Bitboard& b);
  void flatten(const Halfboard& h, Bitboard& b);
  void flatten(const Board& b, Bitboard& c);

  void flip_vertically(Board& b);
  void flip_vertically(Occupancy& o);
  void flip_vertically(Halfboard& h);
  void flip_vertically(Bitboard& b);
}
