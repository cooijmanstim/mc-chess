#pragma once

#include <cstdint>
#include <functional>

typedef uint64_t Bitboard;

// the size_t's here are actually squares::Index, but that would make things circular
namespace bitboard {
  size_t scan_forward(Bitboard b);
  size_t scan_forward_with_reset(Bitboard& b);
  bool is_empty(Bitboard b);
  void for_each_member(Bitboard b, std::function<void(size_t)> f);
  Bitboard flip_vertically(Bitboard b);
}
