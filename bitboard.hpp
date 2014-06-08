#pragma once

#include <cassert>
#include <cstdint>
#include <functional>

#include "bitboard.hpp"

typedef uint64_t Bitboard;

namespace bitboard {
  inline size_t scan_forward(const Bitboard b) {
    assert(b != 0);
    return __builtin_ffsll(b) - 1;
  }

  inline size_t scan_forward_with_reset(Bitboard& b) {
    size_t index = scan_forward(b);
    b &= b - 1;
    return index;
  }

  inline bool is_empty(const Bitboard b) {
    return b == 0;
  }

  template <typename F>
  inline void for_each_member(Bitboard b, F f) {
    while (!is_empty(b)) {
      f(scan_forward_with_reset(b));
    }
  }

  inline Bitboard flip_vertically(const Bitboard b) {
    return __builtin_bswap64(b);
  }

  inline size_t cardinality(const Bitboard b) {
    return __builtin_popcountll(b);
  }
}
