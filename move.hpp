#pragma once

#include <cstdint>
#include <iostream>

#include <boost/optional.hpp>

#include "colors.hpp"
#include "partitions.hpp"
#include "castles.hpp"

class Move {
  typedef uint16_t Word;

  Word move;
  static const size_t nbits_type = 4, nbits_source = 6, nbits_target = 6;
  static const size_t offset_type = 0, offset_source = offset_type + nbits_type, offset_target = offset_source + nbits_source;

public:
  // NOTE: at most 16!
  enum class Type {
    normal,
    double_push,
    castle_kingside,
    castle_queenside,
    capture,
    promotion_knight,
    promotion_bishop,
    promotion_rook,
    promotion_queen,
    capturing_promotion_knight,
    capturing_promotion_bishop,
    capturing_promotion_rook,
    capturing_promotion_queen,
  };

  static std::string typename_from_type(Type type);

  Move();
  Move(const int source, const int target, const Type type);
  Move(const Move& that);

  Move& operator=(const Move& that);

  Type type() const;
  squares::Index source() const;
  squares::Index target() const;

  bool is_capture() const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;
  bool operator< (const Move& that) const; // used for std::set in tests

  bool matches_algebraic(boost::optional<files::Index> source_file, boost::optional<ranks::Index> source_rank, const squares::Index target, const bool is_capture) const;

  static Move castle(Color color, Castle castle);

  friend std::ostream& operator<<(std::ostream& o, const Move& m);
};
