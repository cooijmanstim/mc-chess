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
  Board board;

  // false iff the relevant rook or king has moved.
  std::array<bool, colors::cardinality> can_castle_kingside, can_castle_queenside;

  // if en-passant capture is possible, this is the square where a capturing
  // pawn will end up.
  Bitboard en_passant_square;

  Color color_to_move;

public:
  State();
  State(std::string fen);
  ~State();
  State(State &that);
  bool operator==(State &that);

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  Occupancy occupancy() const;

  std::vector<Move> moves() const;

  Move parse_algebraic(std::string algebraic) const;
  std::vector<Move> match_algebraic(Piece piece,
                                    boost::optional<BoardPartition::Part> source_file,
                                    boost::optional<BoardPartition::Part> source_rank,
                                    bool is_capture,
                                    squares::Index target) const;
  void make_moves(std::string algebraic_variation);
  void make_moves(std::vector<std::string> algebraic_moves);
  void make_move(Move m);
};
