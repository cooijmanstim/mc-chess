#pragma once

#include "bitboard.hpp"

namespace moves {
  void pawn_single_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty);
  void pawn_double_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty);
  void pawn_capture_w(std::vector<Move>& moves, Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square);
  void pawn_capture_e(std::vector<Move>& moves, Bitboard pawn, Bitboard empty, Bitboard them, Bitboard en_passant_square);
  void knight_nnw(std::vector<Move>& moves, Bitboard knight);
  void knight_ssw(std::vector<Move>& moves, Bitboard knight);
  void knight_nww(std::vector<Move>& moves, Bitboard knight);
  void knight_sww(std::vector<Move>& moves, Bitboard knight);
  void knight_nne(std::vector<Move>& moves, Bitboard knight);
  void knight_sse(std::vector<Move>& moves, Bitboard knight);
  void knight_nee(std::vector<Move>& moves, Bitboard knight);
  void knight_see(std::vector<Move>& moves, Bitboard knight);
  void bishop(std::vector<Move>& moves, Bitboard occupancy, squares::Index si);
  void rook(std::vector<Move>& moves, Bitboard occupancy, squares::Index si);
  void queen(std::vector<Move>& moves, Bitboard occupancy, squares::Index si);
  void king(std::vector<Move>& moves, Bitboard king);

  // TODO: promotions, castling
}
