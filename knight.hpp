#pragma once

#include <vector>
#include "bitboard.hpp"

struct KnightAttackType {
  unsigned short leftshift;
  unsigned short rightshift;
  Bitboard badtarget;
};

std::vector<KnightAttackType> get_knight_attack_types();
