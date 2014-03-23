#include "moves.hpp"

// routines for generating moves for White.  flip the bitboards vertically and swap ownership
// to make them applicable for Black.

Move::Move(squares::Index from, squares::Index to, int type) :
  move((type << offset_type) | (from << offset_from) | (to << offset_to))
{
}

Move::Move(const Move& that) :
  move(that.move)
{
}

squares::Index Move::type() const { return (move >> offset_type) & ((1 << nbits_type) - 1); }
squares::Index Move::from() const { return (move >> offset_from) & ((1 << nbits_from) - 1); }
squares::Index Move::to  () const { return (move >> offset_to)   & ((1 << nbits_to)   - 1); }

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

void moves::pawn(std::vector<Move>& moves, Bitboard pawn, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  Bitboard empty = ~(us | them);

  // single push
  bitboard::for_each_member((pawn << north) & empty,
                            [&moves](squares::Index target) {
                              if (ranks::bySquareIndex[target] == ranks::_8) {
                                for (Move::Type promotion: {Move::types::promotion_knight,
                                                            Move::types::promotion_bishop,
                                                            Move::types::promotion_rook,
                                                            Move::types::promotion_queen}) {
                                  moves.push_back(Move(target + directions::south,
                                                       target,
                                                       promotion));
                                }
                              } else {
                                moves.push_back(Move(target + directions::south,
                                                     target));
                              }
                            });

  // double push
  bitboard::for_each_member((pawn << 2*north) & empty,
                            [&moves](squares::Index target) {
                              moves.push_back(Move(target + 2*directions::south,
                                                   target,
                                                   Move::types::double_push));
                            });

  // captures
  moves_from_targets(moves, pawn_attacks_w(pawn) & (them | en_passant_square),
                     directions::south + directions::east, relative);
  moves_from_targets(moves, pawn_attacks_e(pawn) & (them | en_passant_square),
                     directions::south + directions::west, relative);
}

void moves::knight(std::vector<Move>& moves, Bitboard knight, Bitboard us, Bitboard them) {
  moves_from_targets(moves, knight_attacks_nnw(knight) & ~us, 2*directions::south + directions::east,  relative);
  moves_from_targets(moves, knight_attacks_ssw(knight) & ~us, 2*directions::north + directions::east,  relative);
  moves_from_targets(moves, knight_attacks_nww(knight) & ~us, 2*directions::east  + directions::south, relative);
  moves_from_targets(moves, knight_attacks_sww(knight) & ~us, 2*directions::east  + directions::north, relative);
  moves_from_targets(moves, knight_attacks_nne(knight) & ~us, 2*directions::south + directions::west,  relative);
  moves_from_targets(moves, knight_attacks_sse(knight) & ~us, 2*directions::north + directions::west,  relative);
  moves_from_targets(moves, knight_attacks_nee(knight) & ~us, 2*directions::west  + directions::south, relative);
  moves_from_targets(moves, knight_attacks_see(knight) & ~us, 2*directions::west  + directions::north, relative);
}

void moves::bishop(std::vector<Move>& moves, Bitboard bishop, Bitboard us, Bitboard them) {
  for_each_member(bishop, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, bishop_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::rook(std::vector<Move>& moves, Bitboard rook, Bitboard us, Bitboard them) {
  for_each_member(rook, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, rook_attacks(us | them, source) & ~us, source, absolute);
  });
}

void moves::queen(std::vector<Move>& moves, Bitboard queen, Bitboard us, Bitboard them) {
  for_each_member(queen, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, queen_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::king(std::vector<Move>& moves, Bitboard king, Bitboard us) {
  moves_from_targets(moves, king_attacks(king) & ~us, squares::index_from_bitboard(king), absolute);
}


// these assume castling rights have not been lost, i.e., the king and the relevant rook have not moved
void moves::castle_kingside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | f1 | g1, occupancy, attackers))
    moves.push_back(Move(e1, g1, Move::types::castle_kingside));
}

void moves::castle_queenside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | d1 | c1 | b1, occupancy, attackers))
    moves.push_back(Move(e1, c1, Move::types::castle_queenside));
}


void moves::all_moves(std::vector<Move>& moves,
                      array2d<Bitboard, color::cardinality, pieces::cardinality> board,
                      Bitboard en_passant_square) {
  using namespace pieces;

  std::array<Bitboard, colors::cardinality> occupancy = [this](){
    auto occupancy;
    for (Color c: colors::values) {
      occupancy[c] = 0;
      for (Piece c: pieces::values)
        occupancy[c] |= board[c][p];
    }
  }();

  Color us = color::white, them = color::black;
  pawn  (moves, board[us][pawn],   occupancy[us], occupancy[them], en_passant_square);
  knight(moves, board[us][knight], occupancy[us], occupancy[them]);
  bishop(moves, board[us][bishop], occupancy[us], occupancy[them]);
  rook  (moves, board[us][rook],   occupancy[us], occupancy[them]);
  queen (moves, board[us][queen],  occupancy[us], occupancy[them]);
  king  (moves, board[us][king],   occupancy[us], occupancy[them]);
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::name_from_index(m.from()) <<
       "->" << squares::name_from_index(m.to()) <<
       "; " << Move::types::names[m.type()] <<
       ")";
  return o;
}
