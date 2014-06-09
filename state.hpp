#pragma once

#include <array>
#include <boost/optional.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "pieces.hpp"
#include "colored_piece.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "move_generation.hpp"
#include "hash.hpp"
#include "undo.hpp"

class State {
public:
  Color us, them;

  Board board;

  // false iff the relevant rook or king has moved.
  CastlingRights castling_rights;

  // if en-passant capture is possible, this is the square where a capturing
  // pawn will end up.
  Bitboard en_passant_square;

  // number of halfmoves since last capture or pawn move
  // NOTE: does not affect hash
  size_t halfmove_clock;

  // redundant
  Occupancy occupancy;
  Bitboard flat_occupancy;
  Bitboard their_attacks;
  Hash hash;

  State();
  State(std::string fen);
  bool operator==(const State &that) const;

  void empty_board();
  void set_initial_configuration();
  void load_fen(std::string fen);
  std::string dump_fen();

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  boost::optional<ColoredPiece> colored_piece_at(squares::Index square) const;
  Piece piece_at(squares::Index square, Color color) const;
  bool their_king_in_check() const;

  void update_castling_rights(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void update_en_passant_square(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_their_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_our_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_occupancy(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);

  Undo make_move(const Move& move);
  void unmake_move(const Undo& undo);

  void compute_occupancy();
  void compute_their_attacks();
  void compute_hash();

  void compute_occupancy(Occupancy& occupancy, Bitboard& flat_occupancy);
  void compute_their_attacks(Bitboard& their_attacks);
  void compute_hash(Hash &hash);

  bool our_king_captured() const;
  bool game_definitely_over() const;
  bool drawn_by_50() const;
  boost::optional<Color> winner() const;
};
