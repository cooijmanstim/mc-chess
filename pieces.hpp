#pragma once

#include <boost/array.hpp>

namespace pieces {
#define PIECE_TYPES pawn, knight, bishop, rook, queen, king,
  enum Piece { PIECE_TYPES };
  const Piece values[] = { PIECE_TYPES };
#undef PIECE_TYPES

  const size_t cardinality = sizeof(values) / sizeof(Piece);

  const boost::array<char, cardinality> symbols = {
    'p', 'n', 'b', 'r', 'q', 'k',
  };

  Piece type_from_name(std::string name);
}

typedef enum pieces::Piece Piece;
