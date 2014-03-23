#pragma once

#include "util.hpp"
#include "bitboard.hpp"
#include "colors.hpp"
#include "pieces.hpp"

typedef array2d<Bitboard, colors::cardinality, pieces::cardinality> Board;
