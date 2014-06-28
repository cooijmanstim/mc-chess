#pragma once

#include <cstdint>
#include <iostream>

#include <boost/optional.hpp>

#include "colors.hpp"
#include "pieces.hpp"
#include "partitions.hpp"
#include "castles.hpp"
#include "move_types.hpp"

class Move {
  typedef uint16_t Word;

  Word move;
  static const size_t nbits_type = 4, nbits_source = 6, nbits_target = 6;
  static const size_t offset_type = 0, offset_source = offset_type + nbits_type, offset_target = offset_source + nbits_source;

public:

  Move();
  Move(Word move); // from gdb
  Move(const int source, const int target, const MoveType type);
  Move(const Move& that);

  Move& operator=(const Move& that);

  MoveType type() const;
  squares::Index source() const;
  squares::Index target() const;

  bool is_castle() const;
  bool is_capture() const;
  bool is_promotion() const;
  boost::optional<Piece> promotion() const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;

  static Move castle(Color color, Castle castle);

  friend std::ostream& operator<<(std::ostream& o, const Move& m);

  inline friend bool operator<(Move const& a, Move const& b) {
    return a.move < b.move;
  }
};
