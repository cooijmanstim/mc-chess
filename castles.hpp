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
    static array2d<Bitboard, colors::cardinality, castles::cardinality> safe_squares = {
      [white] = {
        [kingside]  = e1 | f1 | g1,
        [queenside] = e1 | d1 | c1,
      },
      [black] = {
        [kingside]  = e8 | f8 | g8,
        [queenside] = e8 | d8 | c8,
      },
    };
    return safe_squares[color][castle];
  }

  inline const Bitboard& free_squares(Color color, Castle castle) {
    using namespace squares;
    using namespace colors;
    static array2d<Bitboard, colors::cardinality, castles::cardinality> free_squares = {
      [white] = {
        [kingside]  = f1 | g1,
        [queenside] = d1 | c1 | b1,
      },
      [black] = {
        [kingside]  = f8 | g8,
        [queenside] = d8 | c8 | b8,
      },
    };
    return free_squares[color][castle];
  }

  inline Move move(Color color, Castle castle) {
    using namespace squares;
    static array2d<Bitboard, colors::cardinality, castles::cardinality> moves = {
      [white] = {
        [kingside]  = Move(e1.index, g1.index, Move::Type::castle_kingside),
        [queenside] = Move(e1.index, c1.index, Move::Type::castle_queenside),
      },
      [black] = {
        [kingside]  = Move(e8.index, g8.index, Move::Type::castle_kingside),
        [queenside] = Move(e8.index, c8.index, Move::Type::castle_queenside),
      },
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
    static array2d<char, colors::cardinality, castles::cardinality> symbols = {
      [white] = {
        [kingside]  = 'K',
        [queenside] = 'Q',
      },
      [black] = {
        [kingside]  = 'k',
        [queenside] = 'q',
      },
    };
    return symbols[color][castle];
  }
}

typedef array2d<bool, colors::cardinality, castles::cardinality> CastlingRights;
