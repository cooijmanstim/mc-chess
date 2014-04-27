#include "colors.hpp"
#include "bitboard.hpp"
#include "partitions.hpp"
#include "moves.hpp"

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

  inline Move move(Color color, Castle castle) {
    using namespace squares;
    static array2d<Move, colors::cardinality, castles::cardinality> moves = {
      // white
      Move(e1, g1, Move::Type::castle_kingside),
      Move(e1, c1, Move::Type::castle_queenside),
      // black
      Move(e8, g8, Move::Type::castle_kingside),
      Move(e8, c8, Move::Type::castle_queenside),
    };
    return moves[color][castle];
  }

  inline const squares::Index& rook_before(const Move& move) {
    using namespace squares;
    switch (move.to()) {
    case g1: return h1;
    case c1: return a1;
    case g8: return h8;
    case c8: return a8;
    default:
      throw std::runtime_error(boost::format("invalid castling move: %1") % move);
    }
  }

  inline const squares::Index& rook_after(const Move& move) {
    using namespace squares;
    switch (move.to()) {
    case g1: return f1;
    case c1: return d1;
    case g8: return f8;
    case c8: return d8;
    default:
      throw std::runtime_error(boost::format("invalid castling move: %1") % move);
    }
  }

  inline boost::optional<Castle> involving(const squares::Index& rook_before_square, const Color color) {
    switch (color) {
    case white:
      switch (rook_before_square) {
      case h1: return kingside;
      case a1: return queenside;
      }
      return boost::none;
    case black:
      switch (rook_before_square) {
      case h8: return kingside;
      case a8: return queenside;
      }
      return boost::none;
    }
  }

  inline char symbol(Color color, Castle castle) {
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

typedef array2d<bool, colors::cardinality, castles::cardinality> CastlingRights;
