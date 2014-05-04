#pragma once

#include <boost/format.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "bitboard.hpp"
#include "partitions.hpp"

namespace castles {
#define CASTLES kingside, queenside
  enum Castle { CASTLES };
  const Castle values[] = { CASTLES };
#undef CASTLES
  const size_t cardinality = sizeof(values) / sizeof(values[0]);

  inline const Bitboard& safe_squares(Color color, Castle castle) {
    using namespace squares::bitboards;
    using namespace colors;
    static auto safe_squares = [](){
      array2d<Bitboard, colors::cardinality, castles::cardinality> result;
      result[white][kingside]  = e1 | f1 | g1;
      result[white][queenside] = e1 | d1 | c1;
      result[black][kingside]  = e8 | f8 | g8;
      result[black][queenside] = e8 | d8 | c8;
      return result;
    }();
    return safe_squares[color][castle];
  }

  inline const Bitboard& free_squares(Color color, Castle castle) {
    using namespace squares::bitboards;
    using namespace colors;
    static auto free_squares = [](){
      array2d<Bitboard, colors::cardinality, castles::cardinality> result;
      result[white][kingside]  = f1 | g1;
      result[white][queenside] = d1 | c1 | b1;
      result[black][kingside]  = f8 | g8;
      result[black][queenside] = d8 | c8 | b8;
      return result;
    }();
    return free_squares[color][castle];
  }

  inline squares::Index king_source(const Color color, const Castle castle) {
    using namespace squares;
    using namespace colors;
    static auto king_sources = [](){
      array2d<squares::Index, colors::cardinality, castles::cardinality> result;
      result[white][kingside]  = e1;
      result[white][queenside] = e1;
      result[black][kingside]  = e8;
      result[black][queenside] = e8;
      return result;
    }();
    return king_sources[color][castle];
  }

  inline squares::Index king_target(const Color color, const Castle castle) {
    using namespace squares;
    using namespace colors;
    static auto king_targets = [](){
      array2d<squares::Index, colors::cardinality, castles::cardinality> result;
      result[white][kingside]  = g1;
      result[white][queenside] = c1;
      result[black][kingside]  = g8;
      result[black][queenside] = c8;
      return result;
    }();
    return king_targets[color][castle];
  }

  inline Color color(const squares::Index king_source) {
    using namespace colors;
    using namespace squares;
    switch (king_source) {
    case e1: return white;
    case e8: return black;
    default:
      throw std::runtime_error(str(boost::format("invalid castling king source: %1%") % king_source));
    }
  }

  inline squares::Index rook_target(const squares::Index king_target) {
    using namespace squares;
    switch (king_target) {
    case g1: return f1;
    case c1: return d1;
    case g8: return f8;
    case c8: return d8;
    default:
      throw std::runtime_error(str(boost::format("invalid castling king target: %1%") % king_target));
    }
  }

  inline squares::Index rook_source(const squares::Index king_target) {
    using namespace squares;
    switch (king_target) {
    case g1: return h1;
    case c1: return a1;
    case g8: return h8;
    case c8: return a8;
    default:
      throw std::runtime_error(str(boost::format("invalid castling king target: %1%") % king_target));
    }
  }

  inline boost::optional<Castle> involving(const squares::Index& rook_source, const Color color) {
    using namespace squares;
    using namespace colors;
    switch (color) {
    case white:
      switch (rook_source) {
      case h1: return kingside;
      case a1: return queenside;
      default: return boost::none;
      }
    case black:
      switch (rook_source) {
      case h8: return kingside;
      case a8: return queenside;
      default: return boost::none;
      }
    }
  }

  inline char symbol(squares::Index king_target) {
    using namespace squares;
    using namespace colors;
    switch (king_target) {
    case g1: return 'K';
    case c1: return 'Q';
    case g8: return 'k';
    case c8: return 'q';
    default:
      throw std::runtime_error(str(boost::format("invalid castling king target: %1%") % king_target));
    }
  }

  inline char symbol(Color color, Castle castle) {
    using namespace squares;
    using namespace colors;
    static auto symbols = []() {
      array2d<char, colors::cardinality, castles::cardinality> result;
      result[white][kingside]  = 'K';
      result[white][queenside] = 'Q';
      result[black][kingside]  = 'k';
      result[black][queenside] = 'q';
      return result;
    }();
    return symbols[color][castle];
  }
}

typedef castles::Castle Castle;
typedef array2d<bool, colors::cardinality, castles::cardinality> CastlingRights;
