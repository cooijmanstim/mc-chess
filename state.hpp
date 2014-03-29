#pragma once

#include <array>
#include <boost/optional.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "pieces.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "moves.hpp"

class State {
  Board board;

  // false iff the relevant rook or king has moved.
  std::array<bool, colors::cardinality> can_castle_kingside, can_castle_queenside;

  // if en-passant capture is possible, this is the square where a capturing
  // pawn will end up.
  Bitboard en_passant_square;

  Color color_to_move;

public:
  State();
  ~State();
  State(State &that);
  bool operator==(State &that);

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  std::vector<Move> moves();
};

