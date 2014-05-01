#pragma once

#include <cstdint>
#include <iostream>

#include <boost/optional.hpp>

#include "colors.hpp"
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
  Move(const int source, const int target, const MoveType type);
  Move(const Move& that);

  Move& operator=(const Move& that);

  MoveType type() const;
  squares::Index source() const;
  squares::Index target() const;

  bool is_capture() const;
  boost::optional<Piece> promotion() const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;
  bool operator< (const Move& that) const; // used for std::set in tests

  bool matches_algebraic(boost::optional<files::Index> source_file, boost::optional<ranks::Index> source_rank, const squares::Index target, const bool is_capture, boost::optional<Piece> promotion) const;

  static Move castle(Color color, Castle castle);

  friend std::ostream& operator<<(std::ostream& o, const Move& m);
};
