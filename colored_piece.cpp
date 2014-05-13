#include "colored_piece.hpp"

ColoredPiece::ColoredPiece(Color color, Piece piece)
  : color(color), piece(piece) {
}

char ColoredPiece::symbol() {
  char symbol = pieces::symbols[piece];
  return color == colors::white
    ? toupper(symbol)
    : tolower(symbol);
}
