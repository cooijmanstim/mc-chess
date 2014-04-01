#include <stdexcept>
#include <cassert>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include "moves.hpp"
#include "direction.hpp"

// routines for generating moves for White.  flip the bitboards vertically and swap ownership
// to make them applicable for Black.

// i fought the language, and the language won.
std::string Move::typename_from_type(Type type) {
  switch (type) {
  case Type::normal:           return "normal";
  case Type::double_push:      return "double_push";
  case Type::castle_kingside:  return "castle_kingside";
  case Type::castle_queenside: return "castle_queenside";
  case Type::capture:          return "capture";
  case Type::promotion_knight: return "promotion_knight";
  case Type::promotion_bishop: return "promotion_bishop";
  case Type::promotion_rook:   return "promotion_rook";
  case Type::promotion_queen:  return "promotion_queen";
  case Type::capturing_promotion_knight: return "capturing_promotion_knight";
  case Type::capturing_promotion_bishop: return "capturing_promotion_bishop";
  case Type::capturing_promotion_rook:   return "capturing_promotion_rook";
  case Type::capturing_promotion_queen:  return "capturing_promotion_queen";
  default: throw std::invalid_argument(str(boost::format("undefined move type: %|1$#x|") % (unsigned short)(type)));
  }
}

Move::Move(squares::Index from, squares::Index to, Type type = Type::normal) :
  move(((Word)type << offset_type) | (from << offset_from) | (to << offset_to))
{
  // TODO: change squares::Index to an unsigned type to catch some more bugs here
  assert(0 <= from && from < squares::partition.cardinality);
  assert(0 <= to   && to   < squares::partition.cardinality);
}

Move::Move(const Move& that) :
  move(that.move)
{
}

Move& Move::operator=(const Move& that) {
  this->move = that.move;
  return *this;
}

Move::Type Move::type() const { return static_cast<Move::Type>((move >> offset_type) & ((1 << nbits_type) - 1)); }
squares::Index Move::from() const { return (move >> offset_from) & ((1 << nbits_from) - 1); }
squares::Index Move::to  () const { return (move >> offset_to)   & ((1 << nbits_to)   - 1); }

bool Move::is_capture() const {
  switch (type()) {
  case Move::Type::capture:
  case Move::Type::capturing_promotion_knight:
  case Move::Type::capturing_promotion_bishop:
  case Move::Type::capturing_promotion_rook:
  case Move::Type::capturing_promotion_queen:
    return true;
  default:
    return false;
  }
}

bool Move::operator==(const Move& that) const { return this->move == that.move; }
bool Move::operator!=(const Move& that) const { return this->move != that.move; }
bool Move::operator< (const Move& that) const { return this->move <  that.move; }

// after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
// FIXME: this is also used for rook attacks along files, for which it does not work
Bitboard slides(Bitboard occupancy, Bitboard piece, Bitboard mobilityMask) {
  Bitboard forward, reverse;
  forward = occupancy & mobilityMask;
  forward -= piece;
  reverse  = __builtin_bswap64(forward);
  reverse -= __builtin_bswap64(piece);
  forward ^= __builtin_bswap64(reverse);
  forward &= mobilityMask;
  return forward;
}

using namespace directions;

Bitboard moves::pawn_attacks_w(Bitboard pawn) { return ((pawn & ~files::a) << vertical >> horizontal); }
Bitboard moves::pawn_attacks_e(Bitboard pawn) { return ((pawn & ~files::h) << vertical << horizontal); }

// FIXME: abstract this some way, pass in left and right shift count to target, and ungood source files
Bitboard moves::knight_attacks_nnw(Bitboard knight) { return (knight & ~files::a)             << 2*vertical >>   horizontal; }
Bitboard moves::knight_attacks_nww(Bitboard knight) { return (knight & ~files::a & ~files::b) <<   vertical >> 2*horizontal; }
Bitboard moves::knight_attacks_nne(Bitboard knight) { return (knight & ~files::h)             << 2*vertical <<   horizontal; }
Bitboard moves::knight_attacks_nee(Bitboard knight) { return (knight & ~files::h & ~files::g) <<   vertical << 2*horizontal; }
Bitboard moves::knight_attacks_ssw(Bitboard knight) { return (knight & ~files::a)             >> 2*vertical >>   horizontal; }
Bitboard moves::knight_attacks_sww(Bitboard knight) { return (knight & ~files::a & ~files::b) >>   vertical >> 2*horizontal; }
Bitboard moves::knight_attacks_sse(Bitboard knight) { return (knight & ~files::h)             >> 2*vertical <<   horizontal; }
Bitboard moves::knight_attacks_see(Bitboard knight) { return (knight & ~files::h & ~files::g) >>   vertical << 2*horizontal; }

Bitboard moves::bishop_attacks(Bitboard occupancy, squares::Index source) {
  return 
    slides(occupancy, squares::partition[source].bitboard,     diagonals::partition.parts_by_square_index[source]) |
    slides(occupancy, squares::partition[source].bitboard, antidiagonals::partition.parts_by_square_index[source]);
}

Bitboard moves::rook_attacks(Bitboard occupancy, squares::Index source) {
  return 
    slides(occupancy, squares::partition[source].bitboard, ranks::partition.parts_by_square_index[source]) |
    slides(occupancy, squares::partition[source].bitboard, files::partition.parts_by_square_index[source]);
}

Bitboard moves::queen_attacks(Bitboard occupancy, squares::Index source) {
  return bishop_attacks(occupancy, source) | rook_attacks(occupancy, source);
}

Bitboard moves::king_attacks(Bitboard king) {
  Bitboard leftright = ((king & ~files::a) >> horizontal) | ((king & ~files::h) << horizontal);
  Bitboard triple = leftright | king;
  return leftright | (triple << vertical) | (triple >> vertical);
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
    king_attacks(attackers[pieces::king]);
}

bool moves::is_attacked(Bitboard targets, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  // TODO: can be done more cheaply; calculate ray attacks from the targets
  return !bitboard::is_empty(all_attacks(occupancy, attackers) & targets);
}


enum Relativity {
  absolute=0, relative = 1
};

// Generate moves from the set of targets.  `source' is the index of the source square, either
// relative to the target or absolute.  The moves are added to the `moves' vector.
void moves_from_targets(std::vector<Move>& moves, Bitboard targets, int source, Relativity relative) {
  bitboard::for_each_member(targets, [&moves, relative, source](squares::Index target) {
      moves.push_back(Move(relative*target + source, target));
    });
}

void moves::pawn(std::vector<Move>& moves, Bitboard pawn, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  Bitboard empty = ~(us | them);

  // single push
  bitboard::for_each_member((pawn << vertical) & empty,
                            [&moves](squares::Index target) {
                              if (ranks::partition.parts_by_square_index[target] == ranks::_8) {
                                for (Move::Type promotion: {Move::Type::promotion_knight,
                                                            Move::Type::promotion_bishop,
                                                            Move::Type::promotion_rook,
                                                            Move::Type::promotion_queen}) {
                                  moves.push_back(Move(target + south,
                                                       target,
                                                       promotion));
                                }
                              } else {
                                moves.push_back(Move(target + south,
                                                     target));
                              }
                            });

  // double push
  bitboard::for_each_member((((pawn << vertical) & empty) << vertical) & empty,
                            [&moves](squares::Index target) {
                              moves.push_back(Move(target + 2*south,
                                                   target,
                                                   Move::Type::double_push));
                            });

  // captures
  // TODO: dry up
  bitboard::for_each_member(pawn_attacks_w(pawn) & (them | en_passant_square),
                            [&moves](squares::Index target) {
                              if (ranks::partition.parts_by_square_index[target] == ranks::_8) {
                                for (Move::Type promotion: {Move::Type::capturing_promotion_knight,
                                                            Move::Type::capturing_promotion_bishop,
                                                            Move::Type::capturing_promotion_rook,
                                                            Move::Type::capturing_promotion_queen}) {
                                  moves.push_back(Move(target + south + east,
                                                       target,
                                                       promotion));
                                }
                              } else {
                                moves.push_back(Move(target + south + east,
                                                     target,
                                                     Move::Type::capture));
                              }
                            });
  bitboard::for_each_member(pawn_attacks_e(pawn) & (them | en_passant_square),
                            [&moves](squares::Index target) {
                              if (ranks::partition.parts_by_square_index[target] == ranks::_8) {
                                for (Move::Type promotion: {Move::Type::capturing_promotion_knight,
                                                            Move::Type::capturing_promotion_bishop,
                                                            Move::Type::capturing_promotion_rook,
                                                            Move::Type::capturing_promotion_queen}) {
                                  moves.push_back(Move(target + south + west,
                                                       target,
                                                       promotion));
                                }
                              } else {
                                moves.push_back(Move(target + south + west,
                                                     target,
                                                     Move::Type::capture));
                              }
                            });
}

void moves::knight(std::vector<Move>& moves, Bitboard knight, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  moves_from_targets(moves, knight_attacks_nnw(knight) & ~us, 2*south + east,  relative);
  moves_from_targets(moves, knight_attacks_ssw(knight) & ~us, 2*north + east,  relative);
  moves_from_targets(moves, knight_attacks_nww(knight) & ~us, 2*east  + south, relative);
  moves_from_targets(moves, knight_attacks_sww(knight) & ~us, 2*east  + north, relative);
  moves_from_targets(moves, knight_attacks_nne(knight) & ~us, 2*south + west,  relative);
  moves_from_targets(moves, knight_attacks_sse(knight) & ~us, 2*north + west,  relative);
  moves_from_targets(moves, knight_attacks_nee(knight) & ~us, 2*west  + south, relative);
  moves_from_targets(moves, knight_attacks_see(knight) & ~us, 2*west  + north, relative);
}

void moves::bishop(std::vector<Move>& moves, Bitboard bishop, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  bitboard::for_each_member(bishop, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, bishop_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::rook(std::vector<Move>& moves, Bitboard rook, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  bitboard::for_each_member(rook, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, rook_attacks(us | them, source) & ~us, source, absolute);
  });
}

void moves::queen(std::vector<Move>& moves, Bitboard queen, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  bitboard::for_each_member(queen, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, queen_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::king(std::vector<Move>& moves, Bitboard king, Bitboard us, Bitboard them, Bitboard en_passant_square) {
  moves_from_targets(moves, king_attacks(king) & ~us, squares::index_from_bitboard(king), absolute);
}


// these assume castling rights have not been lost, i.e., the king and the relevant rook have not moved
// FIXME: never called?
void moves::castle_kingside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | f1 | g1, occupancy, attackers))
    moves.push_back(Move(e1, g1, Move::Type::castle_kingside));
}

void moves::castle_queenside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers) {
  using namespace squares;
  if (!is_attacked(e1 | d1 | c1 | b1, occupancy, attackers))
    moves.push_back(Move(e1, c1, Move::Type::castle_queenside));
}

typedef std::function<void(std::vector<Move>& moves, Bitboard piece, Bitboard us, Bitboard them, Bitboard en_passant_square)> MoveGenerator;

const auto move_generators_by_piece = []() {
  std::vector<MoveGenerator> move_generators = {
    [pieces::pawn]   = moves::pawn,
    [pieces::knight] = moves::knight,
    [pieces::bishop] = moves::bishop,
    [pieces::rook]   = moves::rook,
    [pieces::queen]  = moves::queen,
    [pieces::king]   = moves::king,
  };
  return move_generators;
}();

void moves::piece_moves(std::vector<Move>& moves, Piece piece, Board board, Occupancy occupancy, Bitboard en_passant_square) {
  Color us = colors::white, them = colors::black;
  move_generators_by_piece[piece](moves, board[us][piece], occupancy[us], occupancy[them], en_passant_square);
}

void moves::all_moves(std::vector<Move>& moves, Board board, Occupancy occupancy, Bitboard en_passant_square) {
  Color us = colors::white, them = colors::black;
  for (Piece piece: pieces::values)
    move_generators_by_piece[piece](moves, board[us][piece], occupancy[us], occupancy[them], en_passant_square);
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::partition[m.from()].name <<
       "->" << squares::partition[m.to()].name <<
       "; " << Move::typename_from_type(m.type()) <<
       ")";
  return o;
}
