#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "bitboard.hpp"
#include "partitions.hpp"
#include "board.hpp"
#include "pieces.hpp"

class Move {
  typedef uint16_t Word;

  Word move;
  static const size_t nbits_type = 4, nbits_from = 6, nbits_to = 6;
  static const size_t offset_type = 0, offset_from = offset_type + nbits_type, offset_to = offset_from + nbits_from;

public:
  // NOTE: at most 16!
  enum class Type {
    normal,
    double_push,
    castle_kingside,
    castle_queenside,
    capture,
    promotion_knight,
    promotion_bishop,
    promotion_rook,
    promotion_queen,
    capturing_promotion_knight,
    capturing_promotion_bishop,
    capturing_promotion_rook,
    capturing_promotion_queen,
  };

  static std::string typename_from_type(Type type);

  Move(squares::Index from, squares::Index to, Type type);
  Move(const Move& that);

  Move& operator=(const Move& that);

  Type type() const;
  squares::Index from() const;
  squares::Index to  () const;

  bool is_capture() const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;
  bool operator< (const Move& that) const; // used for std::set in tests

  friend std::ostream& operator<<(std::ostream& o, const Move& m);
};


namespace moves {
  Bitboard slides(Bitboard occupancy, Bitboard piece, Bitboard mobility);
  Bitboard rank_onto_a1h8(Bitboard b, Rank rank);
  Bitboard a1h8_onto_rank(Bitboard b, Rank rank);
  Bitboard slides_rank(Bitboard occupancy, Bitboard piece, Rank rank);

  Bitboard pawn_attacks_w(Bitboard pawn);
  Bitboard pawn_attacks_e(Bitboard pawn);
  Bitboard knight_attacks(Bitboard knight, short leftshift, short rightshift, Bitboard badtarget);
  Bitboard bishop_attacks(Bitboard occupancy, squares::Index source);
  Bitboard rook_attacks(Bitboard occupancy, squares::Index source);
  Bitboard queen_attacks(Bitboard occupancy, squares::Index source);
  Bitboard king_attacks(Bitboard king);
  
  Bitboard all_attacks(Bitboard occupancy, Board board);
  Bitboard black_attacks(Bitboard occupancy, Board board);
  
  void pawn(std::vector<Move>& moves, Bitboard pawn, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void knight(std::vector<Move>& moves, Bitboard knight, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void bishop(std::vector<Move>& moves, Bitboard bishop, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void rook(std::vector<Move>& moves, Bitboard rook, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void queen(std::vector<Move>& moves, Bitboard queen, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void king(std::vector<Move>& moves, Bitboard king, Bitboard us, Bitboard them, Bitboard en_passant_square);
  void castle(std::vector<Move>& moves, Bitboard occupancy, Board board,
              bool can_castle_kingside, bool can_castle_queenside);

  void piece_moves(std::vector<Move>& moves, Piece piece, Board board, Occupancy occupancy, Bitboard en_passant_square);
  void all_moves(std::vector<Move>& moves, Board board, Occupancy occupancy, Bitboard en_passant_square,
                 bool can_castle_kingside, bool can_castle_queenside);
}
