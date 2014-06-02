#include "pieces.hpp"
#include "move.hpp"

#include <boost/format.hpp>

Move::Move() : move(0) {
}

Move::Move(Word move) {
  this->move = move;
}

Move::Move(const int source, const int target, const MoveType type) {
  assert(0 <= source && (size_t)source < squares::cardinality);
  assert(0 <= target && (size_t)target < squares::cardinality);
  move = ((Word)type << offset_type) | (source << offset_source) | (target << offset_target);
}

Move::Move(const Move& that) :
  move(that.move)
{
}

Move& Move::operator=(const Move& that) {
  this->move = that.move;
  return *this;
}

MoveType Move::type() const { return static_cast<MoveType>((move >> offset_type) & ((1 << nbits_type) - 1)); }
squares::Index Move::source() const { return static_cast<squares::Index>((move >> offset_source) & ((1 << nbits_source) - 1)); }
squares::Index Move::target() const { return static_cast<squares::Index>((move >> offset_target)   & ((1 << nbits_target)   - 1)); }

bool Move::is_castle() const {
  switch (type()) {
  case move_types::castle_kingside:
  case move_types::castle_queenside:
    return true;
  default:
    return false;
  }
}

bool Move::is_capture() const {
  switch (type()) {
  case move_types::capture:
  case move_types::king_capture:
  case move_types::capturing_promotion_knight:
  case move_types::capturing_promotion_bishop:
  case move_types::capturing_promotion_rook:
  case move_types::capturing_promotion_queen:
    return true;
  default:
    return false;
  }
}

bool Move::is_king_capture() const {
  return type() == move_types::king_capture;
}

boost::optional<Piece> Move::promotion() const {
  switch (type()) {
#define _(piece) case move_types::promotion_ ## piece: case move_types::capturing_promotion_ ## piece: return pieces:: piece;
    _(knight);
    _(bishop);
    _(rook);
    _(queen);
#undef _
  default:
    return boost::none;
  }
}

bool Move::operator==(const Move& that) const { return this->move == that.move; }
bool Move::operator!=(const Move& that) const { return this->move != that.move; }
bool Move::operator< (const Move& that) const { return this->move <  that.move; }

Move Move::castle(Color color, Castle castle) {
  static auto castle_moves = []() {
    array2d<Move, colors::cardinality, castles::cardinality> result;
    for (Color color: colors::values) {
      for (Castle castle: castles::values) {
        result[color][castle] = Move(king_source(color, castle), king_target(color, castle),
                                     castle == castles::kingside ? move_types::castle_kingside : move_types::castle_queenside);
      }
    }
    return result;
  }();
  return castle_moves[color][castle];
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::keywords[m.source()] <<
       "->" << squares::keywords[m.target()] <<
       "; " << move_types::keywords.at(m.type()) <<
       ")";
  return o;
}

