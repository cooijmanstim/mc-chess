#pragma once

#include <string>
#include <vector>

#include "bitboard.hpp"
#include "partitions.hpp"
#include "pieces.hpp"
#include "board.hpp"
#include "castles.hpp"
#include "move.hpp"

namespace moves {
  // generates pseudolegal moves.  moves that leave the king in check may be
  // generated.  the opponent's only move will be to capture the king.

  void moves(std::vector<Move>& moves, State const& state);
  std::vector<Move> moves(State const& state);
  void king_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void queen_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void rook_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void bishop_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void knight_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void pawn_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void castle_moves(std::vector<Move>& moves, State const& state);
  void legal_moves(std::vector<Move>& moves, State& state);
  void erase_illegal_moves(std::vector<Move>& moves, State& state);
  boost::optional<Move> maybe_fast_random_move(State const& state, boost::mt19937& generator);
  boost::optional<Move> random_move(State const& state, boost::mt19937& generator);
  boost::optional<Move> maybe_make_fast_random_legal_move(State& state, boost::mt19937& generator);
  boost::optional<Move> make_random_legal_move(State& state, boost::mt19937& generator);
}
