#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "bitboard.hpp"
#include "partitions.hpp"
#include "board.hpp"
#include "pieces.hpp"
#include "knight.hpp"

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

  Move(const squares::Index from, const squares::Index to, const Type type);
  Move(const Move& that);

  Move& operator=(const Move& that);

  Type type() const;
  squares::Index from() const;
  squares::Index to  () const;

  bool is_capture() const;

  bool operator==(const Move& that) const;
  bool operator!=(const Move& that) const;
  bool operator< (const Move& that) const; // used for std::set in tests

  bool matches_algebraic(boost::optional<File> source_file, boost::optional<Rank> source_rank, bool is_capture, squares::Index target) const;

  friend std::ostream& operator<<(std::ostream& o, const Move& m);
};


namespace moves {
  Bitboard slides(const Bitboard occupancy, const Bitboard piece, const Bitboard mobility);
  Bitboard slides_rank(const Bitboard occupancy, const Bitboard piece, const Rank rank);

  Bitboard pawn_attacks_w(const Bitboard pawn);
  Bitboard pawn_attacks_e(const Bitboard pawn);
  Bitboard knight_attacks(const Bitboard knight, const KnightAttackType& ka);
  Bitboard bishop_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard rook_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard queen_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard king_attacks(const Bitboard king);

  Bitboard all_attacks(const Bitboard occupancy, const Halfboard& attackers);
  Bitboard black_attacks(const Bitboard occupancy, const Halfboard& attackers);

  void pawn  (std::vector<Move>& moves, const Bitboard pawn,   const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void knight(std::vector<Move>& moves, const Bitboard knight, const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void bishop(std::vector<Move>& moves, const Bitboard bishop, const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void rook  (std::vector<Move>& moves, const Bitboard rook,   const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void queen (std::vector<Move>& moves, const Bitboard queen,  const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void king  (std::vector<Move>& moves, const Bitboard king,   const Bitboard us, const Bitboard them, const Bitboard en_passant_square);
  void castle(std::vector<Move>& moves, const Bitboard occupancy, const Board& board,
              const bool can_castle_kingside, const bool can_castle_queenside);

  void piece_moves(std::vector<Move>& moves, const Piece piece, const Board& board, const Occupancy& occupancy, const Bitboard en_passant_square);
  void all_moves(std::vector<Move>& moves, const Board& board, const Occupancy& occupancy, const Bitboard en_passant_square,
                 const bool can_castle_kingside, const bool can_castle_queenside);
}
