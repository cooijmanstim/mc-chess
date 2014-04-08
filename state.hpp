#pragma once

#include <array>
#include <boost/optional.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "pieces.hpp"
#include "board.hpp"
#include "bitboard.hpp"
#include "moves.hpp"

class State {
public:
  Color us, them;

  Board board;

  // false iff the relevant rook or king has moved.
  std::array<bool, colors::cardinality> can_castle_kingside, can_castle_queenside;

  // if en-passant capture is possible, this is the square where a capturing
  // pawn will end up.
  Bitboard en_passant_square;

  // redundant
  Occupancy occupancy;
  Bitboard their_attacks;

  State();
  State(std::string fen);
  ~State();
  State(State &that);
  bool operator==(const State &that) const;

  void empty_board();

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  std::vector<Move> moves() const;

  Move parse_algebraic(std::string algebraic) const;
  std::vector<Move> match_algebraic(const Piece piece,
                                    boost::optional<File> source_file,
                                    boost::optional<Rank> source_rank,
                                    const Square& target,
                                    const bool is_capture) const;
  void make_moves(std::string algebraic_variation);
  void make_moves(std::vector<std::string> algebraic_moves);
  void make_move(const Move& m);

  void compute_occupancy();
  void compute_attacks();
  void flip_perspective();

  Piece moving_piece(const Move& move, const Halfboard& us) const;
  bool leaves_king_in_check(const Move& m) const;
  void make_move_on_occupancy(const Move& move,
                              const Square& source, const Square& target,
                              Occupancy& occupancy) const;
  void make_move_on_their_halfboard(const Move& move, const Piece piece,
                                    const Square& source, const Square& target,
                                    Halfboard& their_halfboard) const;
  void make_move_on_our_halfboard(const Move& move, const Piece piece,
                                  const Square& source, const Square& target,
                                  Halfboard& our_halfboard) const;
  void update_castling_rights(const Move& move, const Piece piece,
                              const Square& source, const Square& target,
                              bool& can_castle_kingside,
                              bool& can_castle_queenside) const;
  void update_en_passant_square(const Move& move, const Piece piece,
                                const Square& source, const Square& target,
                                Bitboard& en_passant_square) const;
};
