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
    using namespace squares;
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
    using namespace squares;
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
      Move(e1.index, g1.index, Move::Type::castle_kingside),
      Move(e1.index, c1.index, Move::Type::castle_queenside),
      // black
      Move(e8.index, g8.index, Move::Type::castle_kingside),
      Move(e8.index, c8.index, Move::Type::castle_queenside),
    };
    return moves[color][castle];
  }

  inline const Square& rook_before(const Move& move) {
    using namespace squares;
    switch (move.to()) {
    case g1.index: return h1;
    case c1.index: return a1;
    case g8.index: return h8;
    case c8.index: return a8;
    default:
      throw std::runtime_error(boost::format("invalid castling move: %1") % move);
    }
  }

  inline const Square& rook_after(const Move& move) {
    using namespace squares;
    switch (move.to()) {
    case g1.index: return f1;
    case c1.index: return d1;
    case g8.index: return f8;
    case c8.index: return d8;
    default:
      throw std::runtime_error(boost::format("invalid castling move: %1") % move);
    }
  }

  inline boost::optional<Castle> involving(const Square& rook_before_square, const Color color) {
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
