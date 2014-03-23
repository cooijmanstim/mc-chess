#include <cassert>

#include "bitboard.hpp"

squares::Index bitboard::scan_forward(Bitboard b) {
  assert(b != 0);
  return __builtin_ffsll(b) - 1;
}

squares::Index bitboard::scan_forward_with_reset(Bitboard& b) {
  size_t index = bitboard::scan_forward(b);
  b &= b - 1;
  return index;
}

bool bitboard::is_empty(Bitboard b) {
  return b == 0;
}

void bitboard::for_each_member(Bitboard b, std::function<void(squares::Index)> f) {
  while (!is_empty(b)) {
    f(scan_forward_with_reset(b));
  }
}

squares::Index squares::index_from_bitboard(Bitboard b) {
  // assumption that exactly one bit is set
  return bitboard::scan_forward(b);
}

std::string squares::name_from_index(squares::Index i) {
  assert(i < 64);
  return std::string({"abcdefgh"[i & ((1<<3) - 1)],
                      "12345678"[i >> 3]});
}

std::string squares::name_from_bitboard(Bitboard b) {
  return squares::name_from_index(squares::index_from_bitboard(b));
}

