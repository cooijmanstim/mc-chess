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
  void king_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void queen_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void rook_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void bishop_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void knight_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void pawn_moves(std::vector<Move>& moves, State const& state, Bitboard sources);
  void castle_moves(std::vector<Move>& moves, State const& state);
  void capturing(std::vector<Move>& moves, State const& state, squares::Index target, bool to_block_check = false);
  void occupying(std::vector<Move>& moves, State const& state, squares::Index target, bool to_block_check = false);
  void pawn_moves_occupying(std::vector<Move>& moves, State const& state, squares::Index target);
  void pawn_moves_capturing(std::vector<Move>& moves, State const& state, squares::Index target);
  void moves(std::vector<Move>& moves, State const& state);
  void check_evading_moves(std::vector<Move>& moves, State const& state);
  void legal_moves(std::vector<Move>& moves, State& state);
  std::vector<Move> moves(State const& state);
  std::vector<Move> legal_moves(State& state);
  void erase_illegal_moves(std::vector<Move>& moves, State& state);
  boost::optional<Move> maybe_fast_random_move(State const& state, boost::mt19937& generator);
  boost::optional<Move> random_move(State const& state, boost::mt19937& generator);
  boost::optional<Move> maybe_make_fast_random_legal_move(State& state, boost::mt19937& generator);
  boost::optional<Move> make_random_legal_move(State& state, boost::mt19937& generator);
}
