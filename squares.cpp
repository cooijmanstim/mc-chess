#include "squares.hpp"

std::string squares::name_from_bitboard(Bitboard b) {
  // ensure at least one bit set
  assert(b != 0);

  size_t index = 0;
  while ((b & 1) == 0) {
    b >>= 1;
    index++;
  }

  // ensure no more than one bit set
  assert((b >> 1) == 0);

  return std::string({"abcdefgh"[index & ((1<<3) - 1)],
                      "12345678"[index >> 3]});
}
