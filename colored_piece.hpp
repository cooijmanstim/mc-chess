#pragma once

#include "colors.hpp"
#include "pieces.hpp"

struct ColoredPiece {
  Color color;
  Piece piece;

  ColoredPiece(Color color, Piece piece);
  char symbol();
};
