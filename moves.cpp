#include <stdexcept>
#include <cassert>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include "moves.hpp"
#include "direction.hpp"

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

Move::Move(const squares::Index from, const squares::Index to, const Type type = Type::normal) :
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

bool Move::matches_algebraic(boost::optional<File> source_file,
                             boost::optional<Rank> source_rank,
                             const Square& target,
                             const bool is_capture) const {
  if (source_file && *source_file != files::partition.parts_by_square_index[from()])
    return false;
  if (source_rank && *source_rank != ranks::partition.parts_by_square_index[from()])
    return false;
  if (is_capture && !this->is_capture())
    return false;
  if (to() != target.index)
    return false;
  return true;
}

// after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
Bitboard moves::slides(const Bitboard occupancy, const Bitboard piece, const Bitboard mobility) {
  Bitboard forward, reverse;
  assert((mobility & piece) == 0);
  forward = occupancy & mobility;
  reverse  = __builtin_bswap64(forward);
  forward -= piece;
  reverse -= __builtin_bswap64(piece);
  forward ^= __builtin_bswap64(reverse);
  forward &= mobility;
  return forward;
}

Bitboard rank_onto_a1h8(Bitboard b, const Rank rank) {
  // put the bits for the relevant rank into LSB
  b = (b >> rank.index * directions::vertical) & 0xff;
  // map LSB onto a1h8 diagonal
  b = (b * 0x0101010101010101) & giadonals::a1h8;
  return b;
}

Bitboard a1h8_onto_rank(Bitboard b, const Rank rank) {
  b &= giadonals::a1h8;
  // map diagonal onto MSB
  b *= 0x0101010101010101;
  // down to LSB, dropping everything except MSB
  b /= 0x0100000000000000;
  // shift up to the desired rank
  b <<= rank.index * directions::vertical;
  return b;
}

// like slides, but for attacks by a single rook along a rank.  slides() doesn't work for
// that because the byteswap does not reverse the relevant bits; they are all in the same
// byte.
Bitboard moves::slides_rank(Bitboard occupancy, Bitboard piece, const Rank rank) {
  occupancy = rank_onto_a1h8(occupancy, rank);
  piece     = rank_onto_a1h8(piece,     rank);
  Bitboard attacks = slides(occupancy, piece, giadonals::a1h8 & ~piece);
  return a1h8_onto_rank(attacks, rank);
}

using namespace directions;

const std::vector<moves::PawnDingbat>& moves::get_pawn_dingbats() {
  static std::vector<PawnDingbat> pawn_dingbats = {
    { vertical, 0, ranks::_4.bitboard, ranks::_8.bitboard },
    { 0, vertical, ranks::_5.bitboard, ranks::_1.bitboard },
  };
  return pawn_dingbats;
}

const std::vector<moves::PawnAttackType>& moves::get_pawn_attack_types() {
  static std::vector<PawnAttackType> result = {
    { 0, directions::horizontal, files::h.bitboard },
    { directions::horizontal, 0, files::a.bitboard },
  };
  return result;
}

const std::vector<moves::KnightAttackType>& moves::get_knight_attack_types() {
  using namespace directions;
  using namespace files;
  static std::vector<KnightAttackType> result = {
    {2*vertical +   horizontal,                         0, a.bitboard },
    {               horizontal, 2*vertical               , a.bitboard },
    {2*vertical               ,                horizontal, h.bitboard },
    {                        0, 2*vertical +   horizontal, h.bitboard },
    {  vertical + 2*horizontal,                         0, a | b },
    {             2*horizontal,   vertical               , a | b },
    {  vertical               ,              2*horizontal, g | h },
    {                        0,   vertical + 2*horizontal, g | h },
  };
  return result;
}

Bitboard moves::pawn_attacks(const Bitboard pawn, const PawnDingbat& pd, const PawnAttackType& pa) {
  return (pawn << (pd.leftshift + pa.leftshift) >> (pd.rightshift + pa.rightshift)) & ~pa.badtarget;
}

Bitboard moves::knight_attacks(const Bitboard knight, const KnightAttackType& ka) {
  return (knight << ka.leftshift >> ka.rightshift) & ~ka.badtarget;
}

Bitboard moves::bishop_attacks(const Bitboard occupancy, const squares::Index source) {
  Bitboard piece = squares::partition[source].bitboard;
  return 
    slides(occupancy, piece, diagonals::partition.parts_by_square_index[source] & ~piece) |
    slides(occupancy, piece, giadonals::partition.parts_by_square_index[source] & ~piece);
}

Bitboard moves::rook_attacks(const Bitboard occupancy, const squares::Index source) {
  Bitboard piece = squares::partition[source].bitboard;
  return
    slides     (occupancy, piece, files::partition.parts_by_square_index[source] & ~piece) |
    slides_rank(occupancy, piece, ranks::partition.parts_by_square_index[source]);
}

Bitboard moves::queen_attacks(const Bitboard occupancy, const squares::Index source) {
  return bishop_attacks(occupancy, source) | rook_attacks(occupancy, source);
}

Bitboard moves::king_attacks(const Bitboard king) {
  Bitboard leftright = ((king & ~files::a) >> horizontal) | ((king & ~files::h) << horizontal);
  Bitboard triple = leftright | king;
  return leftright | (triple << vertical) | (triple >> vertical);
}

// NOTE: includes attacks on own pieces
Bitboard moves::attacks(const Color us, const Bitboard occupancy, const Halfboard& attackers) {
  Bitboard attacks = 0;

  for (PawnAttackType pa: get_pawn_attack_types())
    attacks |= pawn_attacks(attackers[pieces::pawn], get_pawn_dingbats()[us], pa);

  for (KnightAttackType ka: get_knight_attack_types())
    attacks |= knight_attacks(attackers[pieces::knight], ka);

  // TODO: dry up
  bitboard::for_each_member(attackers[pieces::bishop], [&attacks, &occupancy](squares::Index source) {
      attacks |= bishop_attacks(occupancy, source);
    });

  bitboard::for_each_member(attackers[pieces::rook], [&attacks, &occupancy](squares::Index source) {
      attacks |= rook_attacks(occupancy, source);
    });

  bitboard::for_each_member(attackers[pieces::queen], [&attacks, &occupancy](squares::Index source) {
      attacks |= queen_attacks(occupancy, source);
    });

  return attacks | king_attacks(attackers[pieces::king]);
}

enum Relativity {
  absolute=0, relative = 1
};

// Generate moves from the set of targets.  `source' is the index of the source square, either
// relative to the target or absolute.  The moves are added to the `moves' vector.
void moves_from_targets(std::vector<Move>& moves, const Bitboard targets, const int source, const Relativity relative, const Move::Type type) {
  bitboard::for_each_member(targets, [&moves, &relative, &source, &type](const squares::Index target) {
      moves.push_back(Move(relative*target + source, target, type));
    });
}

// Generate normal and capturing moves from the set of attacks.
void moves_from_attacks(std::vector<Move>& moves, const Bitboard attacks,
                        const Bitboard us, const Bitboard them,
                        const int source, const Relativity relativity) {
  moves_from_targets(moves, attacks & ~us & ~them, source, relativity, Move::Type::normal);
  assert((us & them) == 0);
  moves_from_targets(moves, attacks & them, source, relativity, Move::Type::capture);
}


void moves::pawn(std::vector<Move>& moves,
                 const Color us, const Color them,
                 const Bitboard pawn, const Occupancy& occupancy,
                 const Bitboard en_passant_square) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);

  const PawnDingbat &pd = get_pawn_dingbats()[us];
  
  short direction = pd.leftshift - pd.rightshift;

#define FORWARD(pawn) ((pawn << pd.leftshift >> pd.rightshift))
  // single push
  Bitboard single_push_targets = FORWARD(pawn) & ~flat_occupancy;
  moves_from_targets(moves, single_push_targets & ~pd.promotion_rank, -direction, relative, Move::Type::normal);
  Bitboard promotion_targets = single_push_targets & pd.promotion_rank;
  if (promotion_targets != 0) {
    for (Move::Type promotion: {Move::Type::promotion_knight, Move::Type::promotion_bishop,
                                Move::Type::promotion_rook,   Move::Type::promotion_queen}) {
      moves_from_targets(moves, promotion_targets, -direction, relative, promotion);
    }
  }

  // double push
  Bitboard double_push_targets = FORWARD(FORWARD(pawn)) & pd.double_push_target_rank & ~flat_occupancy;
  moves_from_targets(moves, double_push_targets, -2*direction, relative, Move::Type::double_push);
#undef FORWARD

  // captures
  for (const PawnAttackType& pa: get_pawn_attack_types()) {
    direction = pd.leftshift - pd.rightshift + pa.leftshift - pa.rightshift;

    Bitboard capture_targets = pawn_attacks(pawn, pd, pa) & (occupancy[them] | en_passant_square);
    moves_from_targets(moves, capture_targets & ~pd.promotion_rank, -direction, relative, Move::Type::capture);
    promotion_targets = capture_targets & pd.promotion_rank;
    if (promotion_targets != 0) {
      for (Move::Type promotion: {Move::Type::capturing_promotion_knight, Move::Type::capturing_promotion_bishop,
                                  Move::Type::capturing_promotion_rook,   Move::Type::capturing_promotion_queen}) {
        moves_from_targets(moves, promotion_targets, -direction, relative, promotion);
      }
    }
  }
}

void moves::knight(std::vector<Move>& moves,
                   const Color us, const Color them,
                   const Bitboard knight, const Occupancy& occupancy,
                   const Bitboard en_passant_square) {
  for (KnightAttackType ka: get_knight_attack_types()) {
    moves_from_attacks(moves, knight_attacks(knight, ka),
                       occupancy[us], occupancy[them], -(ka.leftshift - ka.rightshift), relative);
  }
}

void moves::bishop(std::vector<Move>& moves,
                   const Color us, const Color them,
                   const Bitboard bishop, const Occupancy& occupancy,
                   const Bitboard en_passant_square) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  bitboard::for_each_member(bishop, [&moves, &us, &them, &occupancy, &flat_occupancy](const squares::Index source) {
      moves_from_attacks(moves, bishop_attacks(flat_occupancy, source), occupancy[us], occupancy[them], source, absolute);
    });
}

void moves::rook(std::vector<Move>& moves,
                 const Color us, const Color them,
                 const Bitboard rook, const Occupancy& occupancy,
                 const Bitboard en_passant_square) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  bitboard::for_each_member(rook, [&moves, &us, &them, &occupancy, &flat_occupancy](squares::Index source) {
      moves_from_attacks(moves, rook_attacks(flat_occupancy, source), occupancy[us], occupancy[them], source, absolute);
    });
}

void moves::queen(std::vector<Move>& moves,
                  const Color us, const Color them,
                  const Bitboard queen, const Occupancy& occupancy,
                  const Bitboard en_passant_square) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  bitboard::for_each_member(queen, [&moves, &us, &them, &occupancy, &flat_occupancy](squares::Index source) {
      moves_from_attacks(moves, queen_attacks(flat_occupancy, source), occupancy[us], occupancy[them], source, absolute);
    });
}

void moves::king(std::vector<Move>& moves,
                 const Color us, const Color them,
                 const Bitboard king, const Occupancy& occupancy,
                 const Bitboard en_passant_square) {
  moves_from_attacks(moves, king_attacks(king), occupancy[us], occupancy[them], squares::index_from_bitboard(king), absolute);
}

// NOTE: can_castle_{king,queen}side convey that castling rights have not been lost,
// i.e., the king and the relevant rook have not moved
void moves::castle(std::vector<Move>& moves,
                   const Color us, const Color them,
                   const Occupancy& occupancy,
                   const Bitboard their_attacks,
                   const bool can_castle_kingside, const bool can_castle_queenside) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);

  using namespace squares;

  if (us == colors::white) {
    if (can_castle_kingside &&
        !(their_attacks & (e1 | f1 | g1)) &&
        !(flat_occupancy & (f1 | g1)))
      moves.push_back(Move(e1.index, g1.index, Move::Type::castle_kingside));
    if (can_castle_queenside &&
        !(their_attacks & (e1 | d1 | c1 | b1)) &&
        !(flat_occupancy & (d1 | c1 | b1)))
      moves.push_back(Move(e1.index, c1.index, Move::Type::castle_queenside));
  } else {
    if (can_castle_kingside &&
        !(their_attacks & (e8 | f8 | g8)) &&
        !(flat_occupancy & (f8 | g8)))
      moves.push_back(Move(e8.index, g8.index, Move::Type::castle_kingside));
    if (can_castle_queenside &&
        !(their_attacks & (e8 | d8 | c8 | b8)) &&
        !(flat_occupancy & (d8 | c8 | b8)))
      moves.push_back(Move(e8.index, c8.index, Move::Type::castle_queenside));
  }
}

void moves::moves(std::vector<Move>& moves,
                  const Color us, const Color them,
                  const Board& board, const Occupancy& occupancy,
                  const Bitboard their_attacks,
                  const Bitboard en_passant_square,
                  const bool can_castle_kingside, const bool can_castle_queenside) {
#define GENERATE_MOVES(piece) moves::piece(moves, us, them, board[us][pieces::piece], occupancy, en_passant_square)
  GENERATE_MOVES(pawn);
  GENERATE_MOVES(knight);
  GENERATE_MOVES(bishop);
  GENERATE_MOVES(rook);
  GENERATE_MOVES(queen);
#undef GENERATE_MOVES
  castle(moves, us, them, occupancy, their_attacks, can_castle_kingside, can_castle_queenside);
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::partition[m.from()].name <<
       "->" << squares::partition[m.to()].name <<
       "; " << Move::typename_from_type(m.type()) <<
       ")";
  return o;
}
