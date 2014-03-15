#pragma once

namespace pieces {
#define PIECE_TYPES Pawn, Knight, Bishop, Rook, Queen, King,
  enum Piece { PIECE_TYPES };
  const Piece values[] = { PIECE_TYPES };
#undef PIECE_TYPES

  const size_t cardinality = sizeof(values) / sizeof(Piece);
}

typedef enum pieces::Piece Piece;
