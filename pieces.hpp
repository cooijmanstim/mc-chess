#pragma once

#include <map>

#include <boost/assign/list_of.hpp>

namespace pieces {
#define PIECE_TYPES pawn, knight, bishop, rook, queen, king,
  enum Piece { PIECE_TYPES };
  const Piece values[] = { PIECE_TYPES };
#undef PIECE_TYPES

  const size_t cardinality = sizeof(values) / sizeof(Piece);

  std::map<std::string, Piece> by_name = {
    {"pawn", pawn},
    {"knight", knight},
    {"bishop", bishop},
    {"rook", queen},
    {"king", king},
  };

  Piece type_from_name(std::string name) { return by_name[name]; }
}

typedef enum pieces::Piece Piece;
