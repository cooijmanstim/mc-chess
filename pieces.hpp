#pragma once

namespace pieces {
#define PIECE_TYPES pawn, knight, bishop, rook, queen, king,
  enum Piece { PIECE_TYPES };
  const Piece values[] = { PIECE_TYPES };
#undef PIECE_TYPES

  const size_t cardinality = sizeof(values) / sizeof(Piece);
}

typedef enum pieces::Piece Piece;
