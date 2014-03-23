#pragma once

#include "bitboard.hpp"

class Move {
  const uint16_t move;
  static const size_t nbits_type = 4, nbits_from = 6, nbits_to = 6;
  static const size_t offset_type = 0, offset_from = offset_type + nbits_type, offset_to = offset_from + nbits_from;

  // NOTE: at most 16!
  namespace types {
    enum Type {
      normal,
      double_push,
      castle_kingside,
      castle_queenside,
      promotion_knight,
      promotion_bishop,
      promotion_rook,
      promotion_queen,
    };

    std::string names[] = {
      "normal",
      "double_push",
      "castle_kingside",
      "castle_queenside",
      "promotion_knight",
      "promotion_bishop",
      "promotion_rook",
      "promotion_queen",
    };
  }
  typedef types::Type Type;

  Move(squares::Index from, squares::Index to, int type);
  Move(const Move& that);

  squares::Index type() const;
  squares::Index from() const;
  squares::Index to  () const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;
  bool operator< (const Move& that) const; // used for std::set in tests

  friend std::ostream& operator<<(std::ostream& o, const Move& b);
}

namespace moves {
  Bitboard moves::pawn_attacks_w(Bitboard pawn);
  Bitboard moves::pawn_attacks_e(Bitboard pawn);
  Bitboard moves::knight_attacks_nnw(Bitboard knight);
  Bitboard moves::knight_attacks_ssw(Bitboard knight);
  Bitboard moves::knight_attacks_nww(Bitboard knight);
  Bitboard moves::knight_attacks_sww(Bitboard knight);
  Bitboard moves::knight_attacks_nne(Bitboard knight);
  Bitboard moves::knight_attacks_sse(Bitboard knight);
  Bitboard moves::knight_attacks_nee(Bitboard knight);
  Bitboard moves::knight_attacks_see(Bitboard knight);
  Bitboard moves::bishop_attacks(Bitboard occupancy, squares::Index source);
  Bitboard moves::rook_attacks(Bitboard occupancy, squares::Index source);
  Bitboard moves::queen_attacks(Bitboard occupancy, squares::Index source);
  Bitboard moves::king_attacks(Bitboard king);
  
  Bitboard moves::all_attacks(Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers);
  bool moves::is_attacked(Bitboard targets, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers);
  
  void moves::pawn_single_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty);
  void moves::pawn_double_push(std::vector<Move>& moves, Bitboard pawn, Bitboard empty);
  void moves::pawn_capture_w(std::vector<Move>& moves, Bitboard pawn, Bitboard them, Bitboard en_passant_square);;
  void moves::pawn_capture_e(std::vector<Move>& moves, Bitboard pawn, Bitboard them, Bitboard en_passant_square);
  void moves::knight(std::vector<Move>& moves, Bitboard knight);
  void moves::bishop(std::vector<Move>& moves, Bitboard occupancy, squares::Index source);
  void moves::rook(std::vector<Move>& moves, Bitboard occupancy, squares::Index source);
  void moves::queen(std::vector<Move>& moves, Bitboard occupancy, squares::Index source);
  void moves::king(std::vector<Move>& moves, Bitboard king);
  void moves::castle_kingside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers);
  void moves::castle_queenside(std::vector<Move>& moves, Bitboard occupancy, std::array<Bitboard, pieces::cardinality> attackers);

  void moves::all_moves(std::vector<Move>& moves, Bitboard occupancy, array2d<Bitboard, colors::cardinality, pieces::cardinality> board);
}
