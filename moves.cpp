#include <stdexcept>
#include <cassert>

#include "moves.hpp"

// routines for generating moves for White.  flip the bitboards vertically and swap ownership
// to make them applicable for Black.

// i fought the language, and the language won.
std::string Move::typename_from_type(Type type) {
  switch (type) {
  case Type::normal:           return "normal";
  case Type::double_push:      return "double_push";
  case Type::castle_kingside:  return "castle_kingside";
  case Type::castle_queenside: return "castle_queenside";
  case Type::promotion_knight: return "promotion_knight";
  case Type::promotion_bishop: return "promotion_bishop";
  case Type::promotion_rook:   return "promotion_rook";
  case Type::promotion_queen:  return "promotion_queen";
  default: throw std::invalid_argument("undefined move type");
  }
}

Move::Move(squares::Index from, squares::Index to, Type type = Type::normal) :
  move(((Word)type << offset_type) | (from << offset_from) | (to << offset_to))
{
  // TODO: change squares::Index to an unsigned type to catch some more bugs here
  assert(0 <= from && from < squares::cardinality);
  assert(0 <= to   && to   < squares::cardinality);
}

Move::Move(std::string algebraic, Board board) :
  move(parse_algebraic(algebraic, board))
{
}

Move::Move(const Move& that) :
  move(that.move)
{
}

Move::Type Move::type() const { return static_cast<Move::Type>((move >> offset_type) & ((1 << nbits_type) - 1)); }
squares::Index Move::from() const { return (move >> offset_from) & ((1 << nbits_from) - 1); }
squares::Index Move::to  () const { return (move >> offset_to)   & ((1 << nbits_to)   - 1); }

Move::Word Move::parse_algebraic(std::string algebraic, Board board, Bitboard en_passant_square) {
  boost::regex algebraic_move_regex("([A-H]?)([a-h]?)([1-8]?)(x?)([a-h][1-8])");
  boost::smatch m;
  if (!boost::regex_match(algebraic, m, algebraic_move_regex))
    throw std::runtime_error(boost::format("can't parse algebraic move: %1") % algebraic);

  Piece piece = pieces::type_from_name(std::string(m[1].first, m[1].last));

  Bitboard source_file = 0, source_rank = 0;
  if (m[2].matched) source_file = files::bitboard_from_name(std::string(m[2].first, m[2].last));
  if (m[3].matched) source_rank = ranks::bitboard_from_name(std::string(m[3].first, m[3].last));

  bool is_capture = m[4].matched;

  squares::Index target = squares::index_from_name(std::string(m[5].first, m[5].second));

  std::vector<Move> candidates;
  piece_moves(candidates, piece, board, en_passant_square);

  std::remove_if(candidates.begin(), candidates.end(), [file, rank, is_capture](const Move& candidate) {
      if (!bitboard::is_empty(source_file) &&
          source_file != files::bitboard_from_square_index(candidate.from()))
        return true;
      if (!bitboard::is_empty(source_rank) &&
          source_rank != ranks::bitboard_from_square_index(candidate.from()))
        return true;
      if (is_capture && candidate.type != Move::Type::capture)
        return true;
      if (candidate.to() != target)
        return true;
      return false;
    });

  if (candidates.is_empty())
    throw std::runtime_error(boost::format("can't find match for algebraic move: %1") % algebraic);

  if (candidates.size() > 1)
    throw std::runtime_error(boost::format("ambiguous algebraic move: %1, candidates: %2") % algebraic % candidates);

  return candidates[0];
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

Bitboard moves::knight_attacks_nnw(Bitboard knight) { return (knight & ~files::a)             << 2*vertical >>   horizontal; }
Bitboard moves::knight_attacks_nww(Bitboard knight) { return (knight & ~files::a & ~files::b) <<   vertical >> 2*horizontal; }
Bitboard moves::knight_attacks_nne(Bitboard knight) { return (knight & ~files::h)             << 2*vertical <<   horizontal; }
Bitboard moves::knight_attacks_nee(Bitboard knight) { return (knight & ~files::h & ~files::g) <<   vertical << 2*horizontal; }

// NOTE: shifting in the other direction to avoid negative shifts
Bitboard moves::knight_attacks_ssw(Bitboard knight) { return (knight & ~files::a)             >> 2*vertical >>   horizontal; }
Bitboard moves::knight_attacks_sww(Bitboard knight) { return (knight & ~files::a & ~files::b) >>   vertical >> 2*horizontal; }
Bitboard moves::knight_attacks_sse(Bitboard knight) { return (knight & ~files::h)             >> 2*vertical <<   horizontal; }
Bitboard moves::knight_attacks_see(Bitboard knight) { return (knight & ~files::h & ~files::g) >>   vertical << 2*horizontal; }

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
                              if (ranks::bySquareIndex[target] == ranks::_8) {
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
  bitboard::for_each_member((pawn << 2*vertical) & empty,
                            [&moves](squares::Index target) {
                              moves.push_back(Move(target + 2*south,
                                                   target,
                                                   Move::Type::double_push));
                            });

  // captures
  moves_from_targets(moves, pawn_attacks_w(pawn) & (them | en_passant_square),
                     south + east, relative);
  moves_from_targets(moves, pawn_attacks_e(pawn) & (them | en_passant_square),
                     south + west, relative);
}

void moves::knight(std::vector<Move>& moves, Bitboard knight, Bitboard us, Bitboard them) {
  moves_from_targets(moves, knight_attacks_nnw(knight) & ~us, 2*south + east,  relative);
  moves_from_targets(moves, knight_attacks_ssw(knight) & ~us, 2*north + east,  relative);
  moves_from_targets(moves, knight_attacks_nww(knight) & ~us, 2*east  + south, relative);
  moves_from_targets(moves, knight_attacks_sww(knight) & ~us, 2*east  + north, relative);
  moves_from_targets(moves, knight_attacks_nne(knight) & ~us, 2*south + west,  relative);
  moves_from_targets(moves, knight_attacks_sse(knight) & ~us, 2*north + west,  relative);
  moves_from_targets(moves, knight_attacks_nee(knight) & ~us, 2*west  + south, relative);
  moves_from_targets(moves, knight_attacks_see(knight) & ~us, 2*west  + north, relative);
}

void moves::bishop(std::vector<Move>& moves, Bitboard bishop, Bitboard us, Bitboard them) {
  bitboard::for_each_member(bishop, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, bishop_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::rook(std::vector<Move>& moves, Bitboard rook, Bitboard us, Bitboard them) {
  bitboard::for_each_member(rook, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, rook_attacks(us | them, source) & ~us, source, absolute);
  });
}

void moves::queen(std::vector<Move>& moves, Bitboard queen, Bitboard us, Bitboard them) {
  bitboard::for_each_member(queen, [&moves, us, them](squares::Index source) {
      moves_from_targets(moves, queen_attacks(us | them, source) & ~us, source, absolute);
    });
}

void moves::king(std::vector<Move>& moves, Bitboard king, Bitboard us, Bitboard them) {
  moves_from_targets(moves, king_attacks(king) & ~us, squares::index_from_bitboard(king), absolute);
}


// these assume castling rights have not been lost, i.e., the king and the relevant rook have not moved
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


void moves::all_moves(std::vector<Move>& moves, Board board, Bitboard en_passant_square) {
  auto occupancy = [&board](){
    std::array<Bitboard, colors::cardinality> occupancy;
    for (Color c: colors::values) {
      occupancy[c] = 0;
      for (Piece p: pieces::values)
        occupancy[c] |= board[c][p];
    }
    return occupancy;
  }();

  Color us = colors::white, them = colors::black;
  pawn  (moves, board[us][pieces::pawn],   occupancy[us], occupancy[them], en_passant_square);
  knight(moves, board[us][pieces::knight], occupancy[us], occupancy[them]);
  bishop(moves, board[us][pieces::bishop], occupancy[us], occupancy[them]);
  rook  (moves, board[us][pieces::rook],   occupancy[us], occupancy[them]);
  queen (moves, board[us][pieces::queen],  occupancy[us], occupancy[them]);
  king  (moves, board[us][pieces::king],   occupancy[us], occupancy[them]);
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::name_from_index(m.from()) <<
       "->" << squares::name_from_index(m.to()) <<
       "; " << Move::typename_from_type(m.type()) <<
       ")";
  return o;
}
