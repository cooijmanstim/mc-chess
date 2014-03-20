#pragma once

#include "bitboard.hpp"

namespace moves {

  Bitboard pawn_single_push(Bitboard pawn, Bitboard empty) { return (pawn <<   north) & empty; }
  Bitboard pawn_double_push(Bitboard pawn, Bitboard empty) { return (pawn << 2*north) & empty; }
  
  Bitboard pawn_capture_w(Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square) {
    return ((pawn & ~bitboards::files::a) << (north + west)) & (them | en_passant_square);
  }
  
  Bitboard pawn_capture_e(Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square) {
    return ((pawn & ~bitboards::files::h) << (north + east)) & (them | en_passant_square);
  }
  
  Bitboard knight_nnw(Bitboard knight) { return (knight & ~bitboards::files::a)                        << (north + north + west); }
  Bitboard knight_ssw(Bitboard knight) { return (knight & ~bitboards::files::a)                        << (south + south + west); }
  Bitboard knight_nww(Bitboard knight) { return (knight & ~bitboards::files::a & ~bitboards::files::b) << (north + west  + west); }
  Bitboard knight_sww(Bitboard knight) { return (knight & ~bitboards::files::a & ~bitboards::files::b) << (south + west  + west); }
  Bitboard knight_nne(Bitboard knight) { return (knight & ~bitboards::files::h)                        << (north + north + east); }
  Bitboard knight_sse(Bitboard knight) { return (knight & ~bitboards::files::h)                        << (south + south + east); }
  Bitboard knight_nee(Bitboard knight) { return (knight & ~bitboards::files::h & ~bitboards::files::g) << (north + east  + east); }
  Bitboard knight_see(Bitboard knight) { return (knight & ~bitboards::files::h & ~bitboards::files::g) << (south + east  + east); }

  Bitboard sliding(SquareMagic table[], Bitboard occupancy, SquareMagic si) {
    SquareMagic sm = table[si];
    size_t key = ((occupancy & sm.mask) * sm.magic) >> sm.nbits;
    return table[sq].attack_table[key];
  }
  
  Bitboard bishop(Bitboard occupancy, SquareIndex si) {
    return sliding_moves(bishop_table, occupancy, si);
  }
  
  Bitboard rook(Bitboard occupancy, SquareIndex si) {
    return sliding_moves(rook_table, occupancy, si);
  }
  
  Bitboard queen(Bitboard occupancy, SquareIndex si) {
    return bishop_moves(occupancy, si) | rook_moves(occupancy, si);
  }
  
  Bitboard king(Bitboard king) {
    Bitboard poop = ((king & ~files::a) << west) | ((king & ~files::h) << east);
    Bitboard triple = (poop | king);
    poop |= (triple << north) | (triple << south);
  }
}
