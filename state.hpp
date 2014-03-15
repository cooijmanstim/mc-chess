#pragma once

#include <array>
#include <boost/optional.hpp>

#include "colors.hpp"
#include "pieces.hpp"
#include "bitboard.hpp"

class State {
  std::array<std::array<Bitboard, pieces.cardinality>, colors.cardinality> board;

  // false iff the relevant rook or king has moved.
  std::array<bool, colors.cardinality> can_castle_kingside, can_castle_queenside;

  // if en-passant capture is possible, this is the square where a capturing
  // pawn will end up.
  boost::optional<size_t> en_passant_square;

  Color color_to_move;

  State();
  ~State();
  State(State &that);
  bool operator==(State &that);
};

