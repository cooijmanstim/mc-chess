#include "move.hpp"

#include <boost/format.hpp>

Move::Move() : move(0) {
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

bool Move::is_capture() const {
  switch (type()) {
  case move_types::capture:
  case move_types::capturing_promotion_knight:
  case move_types::capturing_promotion_bishop:
  case move_types::capturing_promotion_rook:
  case move_types::capturing_promotion_queen:
    return true;
  default:
    return false;
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


bool Move::matches_algebraic(boost::optional<files::Index> source_file,
                             boost::optional<ranks::Index> source_rank,
                             const squares::Index target,
                             const bool is_capture) const {
  if (source_file && *source_file != files::by_square(source()))
    return false;
  if (source_rank && *source_rank != ranks::by_square(source()))
    return false;
  if (is_capture && !this->is_capture())
    return false;
  if (target != this->target())
    return false;
  return true;
}

std::ostream& operator<<(std::ostream& o, const Move& m) {
  o << "Move(" << squares::keywords[m.source()] <<
       "->" << squares::keywords[m.target()] <<
       "; " << move_types::keywords.at(m.type()) <<
       ")";
  return o;
}
