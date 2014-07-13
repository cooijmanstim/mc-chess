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
#include "targets.hpp"

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
  boost::optional<std::string> inconsistency() const;
  void require_consistent() const;

  friend std::ostream& operator<<(std::ostream& o, const State& s);

  void update_castling_rights(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void update_en_passant_square(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_their_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_our_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);
  void make_move_on_occupancy(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target);

  Undo make_move(const Move& move);
  void unmake_move(const Undo& undo);

  inline void compute_occupancy()     { compute_occupancy(occupancy, flat_occupancy); }
  inline void compute_their_attacks() { compute_their_attacks(their_attacks); }
  inline void compute_hash()          { compute_hash(hash); }

  void compute_occupancy(Occupancy& occupancy, Bitboard& flat_occupancy) const;
  void compute_their_attacks(Bitboard& their_attacks) const;
  void compute_hash(Hash &hash) const;

  boost::optional<ColoredPiece> colored_piece_at(squares::Index square) const;
  Piece piece_at(squares::Index square, Color color) const;
  boost::optional<Color> winner() const;

  inline bool can_castle(Castle castle) const {
    return castling_rights[us][castle]
      && !(castles::safe_squares(us, castle) & their_attacks)
      && !(castles::free_squares(us, castle) & flat_occupancy);
  }

  inline bool in_check() const {
    return their_attacks & board[us][pieces::king];
  }

  inline bool their_king_attacked() const {
    return targets::any_attacked(board[them][pieces::king], flat_occupancy, us, board[us]);
  }

  inline bool our_king_captured() const {
    return bitboard::is_empty(board[us][pieces::king]);
  }

  // may return false on games that are over (too expensive to generate moves),
  // but will not return true on games that are not over
  inline bool game_definitely_over() const {
    return drawn_by_50() || our_king_captured();
  }

  inline bool game_over() {
    return game_definitely_over() || moves::legal_moves(*this).empty();
  }

  inline bool drawn_by_50() const {
    return halfmove_clock >= 50;
  }
};
