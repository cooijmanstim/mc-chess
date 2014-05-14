#include "colored_piece.hpp"

ColoredPiece::ColoredPiece(Color color, Piece piece)
  : color(color), piece(piece) {
}

char ColoredPiece::symbol() const {
  char symbol = pieces::symbols[piece];
  return color == colors::white
    ? toupper(symbol)
    : tolower(symbol);
}

bool ColoredPiece::operator==(const ColoredPiece& that) const {
  return this->color == that.color && this->piece == that.piece;
}

bool ColoredPiece::operator!=(const ColoredPiece& that) const {
  return this->color != that.color || this->piece != that.piece;
}
