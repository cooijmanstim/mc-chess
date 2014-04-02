#include <cassert>

#include "bitboard.hpp"

size_t bitboard::scan_forward(Bitboard b) {
  assert(b != 0);
  return __builtin_ffsll(b) - 1;
}

size_t bitboard::scan_forward_with_reset(Bitboard& b) {
  size_t index = bitboard::scan_forward(b);
  b &= b - 1;
  return index;
}

bool bitboard::is_empty(Bitboard b) {
  return b == 0;
}

void bitboard::for_each_member(Bitboard b, std::function<void(size_t)> f) {
  while (!is_empty(b)) {
    f(scan_forward_with_reset(b));
  }
}

Bitboard bitboard::flip_vertically(Bitboard b) {
  return __builtin_bswap64(b);
}
