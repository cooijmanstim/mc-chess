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
  // color-specific pawn stuff
  struct PawnDingbat {
    unsigned short leftshift;
    unsigned short rightshift;
    Bitboard double_push_target_rank;
    Bitboard promotion_rank;
  };

  // pawn attacks (shift is relative; pawn will already be forward-shifted according to PawnDingbat)
  struct PawnAttackType {
    unsigned short leftshift;
    unsigned short rightshift;
    Bitboard badtarget;
  };

  struct KnightAttackType {
    unsigned short leftshift;
    unsigned short rightshift;
    Bitboard badtarget;
  };

  const std::vector<PawnDingbat>& get_pawn_dingbats();
  const std::vector<PawnAttackType>& get_pawn_attack_types();
  const std::vector<KnightAttackType>& get_knight_attack_types();

  Bitboard slides_rank(Bitboard occupancy, Bitboard piece, const ranks::Index rank);
  Bitboard slides(const Bitboard occupancy, const Bitboard piece, const Bitboard mobility);

  Bitboard pawn_attacks(const Bitboard pawn, const PawnDingbat& pd, const PawnAttackType& pa);
  Bitboard knight_attacks(const Bitboard knight, const KnightAttackType& ka);
  Bitboard bishop_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard rook_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard queen_attacks(const Bitboard occupancy, const squares::Index source);
  Bitboard king_attacks(const Bitboard king);
  
  Bitboard attacks(const Color, const Bitboard occupancy, const Halfboard& attackers);

  void pawn  (std::vector<Move>& moves, const Color us, const Color them, const Bitboard pawn,   const Occupancy& occupancy, const Bitboard en_passant_square);
  void knight(std::vector<Move>& moves, const Color us, const Color them, const Bitboard knight, const Occupancy& occupancy, const Bitboard en_passant_square);
  void bishop(std::vector<Move>& moves, const Color us, const Color them, const Bitboard bishop, const Occupancy& occupancy, const Bitboard en_passant_square);
  void rook  (std::vector<Move>& moves, const Color us, const Color them, const Bitboard rook,   const Occupancy& occupancy, const Bitboard en_passant_square);
  void queen (std::vector<Move>& moves, const Color us, const Color them, const Bitboard queen,  const Occupancy& occupancy, const Bitboard en_passant_square);
  void king  (std::vector<Move>& moves, const Color us, const Color them, const Bitboard king,   const Occupancy& occupancy, const Bitboard en_passant_square);
  void castle(std::vector<Move>& moves, const Color us, const Color them,
              const Occupancy& occupancy,
              const Bitboard their_attacks,
              const CastlingRights& castling_rights);

  void moves(std::vector<Move>& moves, const Color us, const Color them,
             const Board& board, const Occupancy& occupancy,
             const Bitboard their_attacks,
             const Bitboard en_passant_square,
             const CastlingRights& castling_rights);
}
