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
  Bitboard their_attacks;
  Hash hash;

  State();
  State(std::string fen);
  ~State();
  State(const State &that);
  bool operator==(const State &that) const;

  void empty_board();
  void set_initial_configuration();
  void load_fen(std::string fen);
  std::string dump_fen();

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  Undo make_move(const Move& m);
  void unmake_move(const Undo& u);

  std::vector<Move> moves() const;
  boost::optional<Move> random_move(boost::mt19937& generator) const;
  std::vector<Move> legal_moves();
  void erase_illegal_moves(std::vector<Move>& moves);
  boost::optional<Move> make_random_legal_move(boost::mt19937& generator);
  bool their_king_in_check() const;

  boost::optional<ColoredPiece> colored_piece_at(squares::Index square) const;
  Piece piece_at(squares::Index square, Color color) const;
  void make_move_on_their_halfboard (const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_our_halfboard   (const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void update_castling_rights       (const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void update_en_passant_square     (const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_occupancy       (const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);

  void compute_occupancy();
  void compute_their_attacks();
  void compute_hash();
  void compute_occupancy(Occupancy& occupancy);
  void compute_their_attacks(Bitboard& their_attacks);
  void compute_hash(Hash& hash);

  bool drawn_by_50() const;
  boost::optional<Color> winner() const;
};
