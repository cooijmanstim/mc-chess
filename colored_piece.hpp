#pragma once

#include "colors.hpp"
#include "pieces.hpp"

struct ColoredPiece {
  Color color;
  Piece piece;

  ColoredPiece(Color color, Piece piece);
  char symbol() const;

  bool operator==(const ColoredPiece& that) const;
  bool operator!=(const ColoredPiece& that) const;
};
