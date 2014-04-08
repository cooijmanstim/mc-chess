#include "board.hpp"

void board::flatten(const Board& b, Occupancy& o) {
  for (Color c: colors::values)
    flatten(b[c], o[c]);
}

void board::flatten(const Board& b, Halfboard& h) {
  for (Piece p: pieces::values)
    flatten(b[p], h[p]);
}

void board::flatten(const Occupancy& o, Bitboard& b) {
  b = 0;
  for (Color c: colors::values)
    b |= o[c];
}

void board::flatten(const Halfboard& h, Bitboard& b) {
  b = 0;
  for (Piece p: pieces::values)
    b |= h[p];
}

void board::flatten(const Board& b, Bitboard& a) {
  a = 0;
  for (Color c: colors::values)
    for (Piece p: pieces::values)
      a |= b[c][p];
}

void board::flip_vertically(Board& b) {
  for (Color c: colors::values)
    flip_vertically(b[c]);
}

void board::flip_vertically(Occupancy& o) {
  for (Color c: colors::values)
    flip_vertically(o[c]);
}

void board::flip_vertically(Halfboard& h) {
  for (Piece p: pieces::values)
    flip_vertically(h[p]);
}

void board::flip_vertically(Bitboard& b) {
  b = bitboard::flip_vertically(b);
}
