// `source' is the index of the source square, relative to target or absolute based on `relative'
void moves::from_targets(std::vector<Move>& moves, Bitboard targets, int source, int relative, int flags=0) {
  while (targets > 0) {
    SquareIndex target = bitboard::scan_forward_with_reset(targets);
    moves.push_back(Move(relative*target + source, target));
  }
}

void moves::pawn_single_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty) {
  Bitboard targets = (pawn << north) & empty;
  from_targets(moves, targets, directions::south, 1);
};

void moves::pawn_double_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty) {
  Bitboard targets = (pawn << 2*north) & empty;
  from_targets(moves, targets, 2*directions::south, 1, Move::flags::double_push);
};

void moves::pawn_capture_w(std::vector<Move>& moves, Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square) {
  Bitboard targets = ((pawn & ~bitboards::files::a) << (north + west)) & (them | en_passant_square);
  from_targets(moves, targets, directions::south + directions::east, 1);
};

void moves::pawn_capture_e(std::vector<Move>& moves, Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square) {
  Bitboard targets = ((pawn & ~bitboards::files::h) << (north + east)) & (them | en_passant_square);
  from_targets(moves, targets, directions::south + directions::west, 1);
};

void moves::knight_nnw(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::a)                        << (north + north + west);
  from_targets(moves, targets, 2*directions::south + directions::east, 1);
}
void moves::knight_ssw(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::a)                        << (south + south + west);
  from_targets(moves, 2*directions::north + directions::east, 1);
}
void moves::knight_nww(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::a & ~bitboards::files::b) << (north + west  + west);
  from_targets(moves, 2*directions::east + directions::south, 1);
}
void moves::knight_sww(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::a & ~bitboards::files::b) << (south + west  + west);
  from_targets(moves, 2*directions::east + directions::north, 1);
}
void moves::knight_nne(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::h)                        << (north + north + east);
  from_targets(moves, 2*directions::south + directions::west, 1);
}
void moves::knight_sse(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::h)                        << (south + south + east);
  from_targets(moves, 2*directions::north + directions::west, 1);
}
void moves::knight_nee(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::h & ~bitboards::files::g) << (north + east  + east);
  from_targets(moves, 2*directions::west + directions::south, 1);
}
void moves::knight_see(std::vector<Move>& moves, Bitboard knight) {
  Bitboard targets = (knight & ~bitboards::files::h & ~bitboards::files::g) << (south + east  + east);
  from_targets(moves, 2*directions::west + directions::north, 1);
}

void moves::bishop(std::vector<Move>& moves, Bitboard occupancy, SquareIndex si) {
  Bitboard targets =
    sliding_moves(occupancy, bitboard::squares::byIndex[si], bitboard::diagonals::bySquareIndex[si]) |
    sliding_moves(occupancy, bitboard::squares::byIndex[si], bitboard::antidiagonals::bySquareIndex[si]);
  from_targets(moves, targets, si, 0);
}

void moves::rook(std::vector<Move>& moves, Bitboard occupancy, SquareIndex si) {
  Bitboard targets =
    sliding_moves(occupancy, bitboard::squares::byIndex[si], bitboard::ranks::bySquareIndex[si]) |
    sliding_moves(occupancy, bitboard::squares::byIndex[si], bitboard::files::bySquareIndex[si]);
  from_targets(moves, targets, si, 0);
}

void moves::queen(std::vector<Move>& moves, Bitboard occupancy, SquareIndex si) {
  moves::bishop(moves, occupancy, si);
  moves::rook(moves, occupancy, si);
}

void moves::king(std::vector<Move>& moves, Bitboard king) {
  Bitboard leftright = ((king & ~files::a) << west) | ((king & ~files::h) << east);
  Bitboard triple = leftright | king;
  Bitboard targets = leftright | (triple << north) | (triple << south);
  SquareIndex si = bitboard::squares::index_from_bitboard(king);
  from_targets(moves, targets, si, 0);
}

// after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
Bitboard moves::slides(Bitboard occupancy, Bitboard piece, Bitboard mobilityMask) {
  Bitboard forward, reverse;
  forward = occ & mobilityMask;
  forward -= piece;
  reverse  = _byteswap_uint64(forward);
  reverse -= _byteswap_uint64(piece);
  forward ^= _byteswap_uint64(reverse);
  forward &= mobilityMask;
  return forward;
}
