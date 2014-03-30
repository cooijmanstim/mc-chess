#include "pieces.hpp"

std::map<std::string, Piece> by_name = {
  {"pawn",   pieces::pawn},
  {"knight", pieces::knight},
  {"bishop", pieces::bishop},
  {"rook",   pieces::queen},
  {"king",   pieces::king},
};

Piece pieces::type_from_name(std::string name) { return by_name[name]; }
