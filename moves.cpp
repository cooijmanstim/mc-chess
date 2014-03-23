// after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
Bitboard slides(Bitboard occupancy, Bitboard piece, Bitboard mobilityMask) {
  Bitboard forward, reverse;
  forward = occ & mobilityMask;
  forward -= piece;
  reverse  = _byteswap_uint64(forward);
  reverse -= _byteswap_uint64(piece);
  forward ^= _byteswap_uint64(reverse);
  forward &= mobilityMask;
  return forward;
}

Bitboard moves::pawn_attack_w(Bitboard pawn) { return ((pawn & ~files::a) << (north + west)); }
Bitboard moves::pawn_attack_e(Bitboard pawn) { return ((pawn & ~files::h) << (north + east)); }

Bitboard moves::knight_attack_nnw(Bitboard knight) { return (knight & ~files::a)             << (north + north + west); }
Bitboard moves::knight_attack_ssw(Bitboard knight) { return (knight & ~files::a)             << (south + south + west); }
Bitboard moves::knight_attack_nww(Bitboard knight) { return (knight & ~files::a & ~files::b) << (north + west  + west); }
Bitboard moves::knight_attack_sww(Bitboard knight) { return (knight & ~files::a & ~files::b) << (south + west  + west); }
Bitboard moves::knight_attack_nne(Bitboard knight) { return (knight & ~files::h)             << (north + north + east); }
Bitboard moves::knight_attack_sse(Bitboard knight) { return (knight & ~files::h)             << (south + south + east); }
Bitboard moves::knight_attack_nee(Bitboard knight) { return (knight & ~files::h & ~files::g) << (north + east  + east); }
Bitboard moves::knight_attack_see(Bitboard knight) { return (knight & ~files::h & ~files::g) << (south + east  + east); }

Bitboard moves::bishop_attacks(Bitboard occupancy, squares::Index source) {
  return 
    slides(occupancy, squares::byIndex[source],     diagonals::bySquareIndex[source]) |
    slides(occupancy, squares::byIndex[source], antidiagonals::bySquareIndex[source]);
}

Bitboard moves::rook_attacks(Bitboard occupancy, squares::Index source) {
  return 
    slides(occupancy, squares::byIndex[source], ranks::bySquareIndex[source]) |
    slides(occupancy, squares::byIndex[source], files::bySquareIndex[source]);
}

Bitboard moves::queen_attacks(Bitboard occupancy, squares::Index source) {
  return bishop_attacks(occupancy, source) | rook_attacks(occupancy, source);
}

Bitboard moves::king_attacks(Bitboard king) {
  Bitboard leftright = ((king & ~files::a) << west) | ((king & ~files::h) << east);
  Bitboard triple = leftright | king;
  return leftright | (triple << north) | (triple << south);
}


enum Relativity {
  absolute=0, relative = 1
};

// Generate moves from the set of targets.  `source' is the index of the source square, either
// relative to the target or absolute.  The moves are added to the `moves' vector.
void moves_from_targets(std::vector<Move>& moves, Bitboard targets, int source, Relativity relative) {
  bitboard::for_each_member(targets, [&moves](squares::Index target) {
      moves.push_back(Move(relative*target + source, target));
    });
}

void moves::pawn_single_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty) {
  bitboard::for_each_member((pawn << north) & empty,
                            [&moves](squares::Index target) {
                              if (ranks::bySquareIndex[target] == ranks::_8) {
                                for (Move::Flag promotion: {Move::flags::promotion_knight,
                                      Move::flags::promotion_bishop,
                                      Move::flags::promotion_rook,
                                      Move::flags::promotion_queen}) {
                                  moves.push_back(Move(target + directions::south, target, promotion));
                                }
                              } else {
                                moves.push_back(Move(target + directions::south, target));
                              }
                            });
}

void moves::pawn_double_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty) {
  bitboard::for_each_member((pawn << 2*north) & empty, [&moves](squares::Index target) {
      moves.push_back(Move(target + 2*directions::south, target, Move::flags::double_push));
    });
}

void moves::pawn_capture_w(std::vector<Move>& moves, Bitboard pawn, Bitboard them, Bitboard en_passant_square) {
  moves_from_targets(moves, pawn_attack_w(pawn) & (them | en_passant_square), directions::south + directions::east, relative);
};

void moves::pawn_capture_e(std::vector<Move>& moves, Bitboard pawn, Bitboard them, Bitboard en_passant_square) {
  moves_from_targets(moves, pawn_attack_e(pawn) & (them | en_passant_square), directions::south + directions::west, relative);
}

void moves::knight(std::vector<Move>& moves, Bitboard knight) {
  moves_from_targets(moves, knight_attack_nnw(knight), 2*directions::south + directions::east,  relative);
  moves_from_targets(moves, knight_attack_ssw(knight), 2*directions::north + directions::east,  relative);
  moves_from_targets(moves, knight_attack_nww(knight), 2*directions::east  + directions::south, relative);
  moves_from_targets(moves, knight_attack_sww(knight), 2*directions::east  + directions::north, relative);
  moves_from_targets(moves, knight_attack_nne(knight), 2*directions::south + directions::west,  relative);
  moves_from_targets(moves, knight_attack_sse(knight), 2*directions::north + directions::west,  relative);
  moves_from_targets(moves, knight_attack_nee(knight), 2*directions::west  + directions::south, relative);
  moves_from_targets(moves, knight_attack_see(knight), 2*directions::west  + directions::north, relative);
}


void moves::bishop(std::vector<Move>& moves, Bitboard occupancy, squares::Index source) {
  moves_from_targets(moves, bishop_attacks(occupancy, source), source, absolute);
}

void moves::rook(std::vector<Move>& moves, Bitboard occupancy, squares::Index source) {
  moves_from_targets(moves, rook_attacks(occupancy, source), source, absolute);
}

void moves::queen(std::vector<Move>& moves, Bitboard occupancy, squares::Index source) {
  moves_from_targets(moves, queen_attacks(occupancy, source), source, absolute);
}

void moves::king(std::vector<Move>& moves, Bitboard king) {
  moves_from_targets(moves, king_attacks(king), squares::index_from_bitboard(king), absolute);
}
