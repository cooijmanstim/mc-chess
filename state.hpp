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

  std::vector<Move> moves() const;
  boost::optional<Move> random_move(boost::mt19937& generator) const;

  void make_moves(std::string algebraic_variation);
  void make_moves(std::vector<std::string> algebraic_moves);
  void make_move(const Move& m);

  boost::optional<ColoredPiece> colored_piece_at(squares::Index square) const;
  Piece moving_piece(const Move& move, const Halfboard& us) const;
  void make_move_on_their_halfboard (const Move& move, const Piece piece, const Bitboard source, const Bitboard target,
                                     Halfboard& their_halfboard,
                                     Hash& hash) const;
  void make_move_on_our_halfboard   (const Move& move, const Piece piece, const Bitboard source, const Bitboard target,
                                     Halfboard& our_halfboard,
                                     Hash& hash) const;
  void update_castling_rights       (const Move& move, const Piece piece, const Bitboard source, const Bitboard target,
                                     CastlingRights& castling_rights,
                                     Hash& hash) const;
  void update_en_passant_square     (const Move& move, const Piece piece, const Bitboard source, const Bitboard target,
                                     Bitboard& en_passant_square,
                                     Hash& hash) const;
  void make_move_on_occupancy       (const Move& move, const Piece piece, const Bitboard source, const Bitboard target,
                                     Occupancy& occupancy,
                                     Hash& hash) const;

  void compute_occupancy();
  void compute_their_attacks();
  void compute_hash();
  void compute_occupancy(Occupancy& occupancy);
  void compute_their_attacks(Bitboard& their_attacks);
  void compute_hash(Hash& hash);

  bool drawn_by_50() const;
  bool our_king_in_check() const;
  boost::optional<Color> winner() const;
};
