#include "moves.hpp"


Move::Move(squares::Index from, squares::Index to, int flags) :
  move((flags << offset_flags) | (from << offset_from) | (to << offset_to))
{
}

Move::Move(const Move& that) :
  move(that.move)
{
}

squares::Index Move::flags() const { return (move >> offset_flags) & ((1 << nbits_flags) - 1); }
squares::Index Move::from () const { return (move >> offset_from)  & ((1 << nbits_from)  - 1); }
squares::Index Move::to   () const { return (move >> offset_to)    & ((1 << nbits_to)    - 1); }

bool Move::operator==(const Move& that) const { return this->move == that.move; }
bool Move::operator!=(const Move& that) const { return this->move != that.move; }


// after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
// FIXME: this is also used for rook attacks along files, for which it does not work
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

Bitboard moves::pawn_attacks_w(Bitboard pawn) { return ((pawn & ~files::a) << (north + west)); }
Bitboard moves::pawn_attacks_e(Bitboard pawn) { return ((pawn & ~files::h) << (north + east)); }

Bitboard moves::knight_attacks_nnw(Bitboard knight) { return (knight & ~files::a)             << (north + north + west); }
Bitboard moves::knight_attacks_ssw(Bitboard knight) { return (knight & ~files::a)             << (south + south + west); }
Bitboard moves::knight_attacks_nww(Bitboard knight) { return (knight & ~files::a & ~files::b) << (north + west  + west); }
Bitboard moves::knight_attacks_sww(Bitboard knight) { return (knight & ~files::a & ~files::b) << (south + west  + west); }
Bitboard moves::knight_attacks_nne(Bitboard knight) { return (knight & ~files::h)             << (north + north + east); }
Bitboard moves::knight_attacks_sse(Bitboard knight) { return (knight & ~files::h)             << (south + south + east); }
Bitboard moves::knight_attacks_nee(Bitboard knight) { return (knight & ~files::h & ~files::g) << (north + east  + east); }
Bitboard moves::knight_attacks_see(Bitboard knight) { return (knight & ~files::h & ~files::g) << (south + east  + east); }

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

Bitboard moves::all_attacks(Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  return
    pawn_attacks_w(attackers[pieces::pawn]) |
    pawn_attacks_e(attackers[pieces::pawn]) |
    knight_attacks_nnw(attackers[pieces::knight]) |
    knight_attacks_ssw(attackers[pieces::knight]) |
    knight_attacks_nww(attackers[pieces::knight]) |
    knight_attacks_sww(attackers[pieces::knight]) |
    knight_attacks_nne(attackers[pieces::knight]) |
    knight_attacks_sse(attackers[pieces::knight]) |
    knight_attacks_nee(attackers[pieces::knight]) |
    knight_attacks_see(attackers[pieces::knight]) |
    bishop_attacks(occupancy, attackers[pieces::bishop]) |
    rook_attacks(occupancy, attackers[pieces::rook]) |
    queen_attacks(occupancy, attackers[pieces::queen]) |
    king_attacks(occupancy, attackers[pieces::king]);
}

bool moves::is_attacked(Bitboard targets, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  // TODO: can be done more cheaply; calculate ray attacks from the targets
  return !is_empty(all_attacks(occupancy, attackers) & targets);
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
  moves_from_targets(moves, pawn_attacks_w(pawn) & (them | en_passant_square), directions::south + directions::east, relative);
}

void moves::pawn_capture_e(std::vector<Move>& moves, Bitboard pawn, Bitboard them, Bitboard en_passant_square) {
  moves_from_targets(moves, pawn_attacks_e(pawn) & (them | en_passant_square), directions::south + directions::west, relative);
}

void moves::knight(std::vector<Move>& moves, Bitboard knight) {
  moves_from_targets(moves, knight_attacks_nnw(knight), 2*directions::south + directions::east,  relative);
  moves_from_targets(moves, knight_attacks_ssw(knight), 2*directions::north + directions::east,  relative);
  moves_from_targets(moves, knight_attacks_nww(knight), 2*directions::east  + directions::south, relative);
  moves_from_targets(moves, knight_attacks_sww(knight), 2*directions::east  + directions::north, relative);
  moves_from_targets(moves, knight_attacks_nne(knight), 2*directions::south + directions::west,  relative);
  moves_from_targets(moves, knight_attacks_sse(knight), 2*directions::north + directions::west,  relative);
  moves_from_targets(moves, knight_attacks_nee(knight), 2*directions::west  + directions::south, relative);
  moves_from_targets(moves, knight_attacks_see(knight), 2*directions::west  + directions::north, relative);
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


// these assume castling rights have not been lost, i.e., the king and the relevant rook have not moved
void moves::castle_kingside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | f1 | g1, occupancy, attackers))
    moves.push_back(Move(e1, g1, Move::flags::castle_kingside));
}

void moves::castle_queenside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | d1 | c1 | b1, occupancy, attackers))
    moves.push_back(Move(e1, c1, Move::flags::castle_queenside));
}
