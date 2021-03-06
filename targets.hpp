#pragma once

#include <vector>

#include "bitboard.hpp"
#include "direction.hpp"
#include "partitions.hpp"

namespace targets {
  // color-specific pawn stuff
  struct PawnDingbat {
    unsigned leftshift;
    unsigned rightshift;
    Bitboard double_push_target_rank;
    Bitboard promotion_rank;

    inline Bitboard single_push_targets(Bitboard piece, Bitboard occupancy) const {
      return piece << leftshift >> rightshift & ~occupancy;
    }

    inline Bitboard double_push_targets(Bitboard piece, Bitboard occupancy) const {
      return single_push_targets(piece, occupancy) << leftshift >> rightshift & ~occupancy & double_push_target_rank;
    }

    inline signed single_push_direction() const {
      return leftshift - rightshift;
    }

    inline signed double_push_direction() const {
      return 2*single_push_direction();
    }
  };

  // pawn attacks (shift is relative; pawn will already be forward-shifted according to PawnDingbat)
  struct PawnAttackType {
    unsigned leftshift;
    unsigned rightshift;
    Bitboard badtarget;
  };

  struct KnightAttackType {
    unsigned leftshift;
    unsigned rightshift;
    Bitboard badtarget;

    inline signed direction() const {
      return leftshift - rightshift;
    }

    inline Bitboard attacks(Bitboard sources) const {
      return (sources << leftshift >> rightshift) & ~badtarget;
    }
  };

  // after https://chessprogramming.wikispaces.com/Hyperbola+Quintessence
  inline Bitboard slides(Bitboard occupancy, Bitboard sources, Bitboard mobility) {
    Bitboard forward, reverse;
    assert((mobility & sources) == 0);
    forward = occupancy & mobility;
    reverse  = __builtin_bswap64(forward);
    forward -= sources;
    reverse -= __builtin_bswap64(sources);
    forward ^= __builtin_bswap64(reverse);
    forward &= mobility;
    return forward;
  }

  inline Bitboard rank_onto_a1h8(Bitboard b, ranks::Index rank) {
    // put the bits for the relevant rank into LSB
    b = (b >> rank * directions::vertical) & 0xff;
    // map LSB onto a1h8 diagonal
    b = (b * 0x0101010101010101) & giadonals::bitboards::a1h8;
    return b;
  }

  inline Bitboard a1h8_onto_rank(Bitboard b, ranks::Index rank) {
    b &= giadonals::bitboards::a1h8;
    // map diagonal onto MSB
    b *= 0x0101010101010101;
    // down to LSB, dropping everything except MSB
    b /= 0x0100000000000000;
    // shift up to the desired rank
    b <<= rank * directions::vertical;
    return b;
  }

  // like slides, but for attacks by a single rook along a rank.  slides() doesn't work for
  // that because the byteswap does not reverse the relevant bits; they are all in the same
  // byte.
  inline Bitboard slides_rank(Bitboard occupancy, Bitboard sources, ranks::Index rank) {
    occupancy = rank_onto_a1h8(occupancy, rank);
    sources   = rank_onto_a1h8(sources,   rank);
    Bitboard attacks = slides(occupancy, sources, giadonals::bitboards::a1h8 & ~sources);
    return a1h8_onto_rank(attacks, rank);
  }

  const PawnDingbat pawn_dingbats[] = {
    { directions::vertical, 0, ranks::bitboards::_4, ranks::bitboards::_8 },
    { 0, directions::vertical, ranks::bitboards::_5, ranks::bitboards::_1 },
  };
    
  const PawnAttackType pawn_attack_types[] = {
    { 0, directions::horizontal, files::bitboards::h },
    { directions::horizontal, 0, files::bitboards::a },
  };
    
  const KnightAttackType knight_attack_types[] = {
    {2*directions::vertical + directions::horizontal, 0, files::bitboards::a},
    {directions::horizontal, 2*directions::vertical, files::bitboards::a},
    {2*directions::vertical, directions::horizontal, files::bitboards::h},
    {0, 2*directions::vertical + directions::horizontal, files::bitboards::h},
    {directions::vertical + 2*directions::horizontal, 0, files::bitboards::a | files::bitboards::b },
    {2*directions::horizontal, directions::vertical, files::bitboards::a | files::bitboards::b },
    {directions::vertical, 2*directions::horizontal, files::bitboards::g | files::bitboards::h },
    {0, directions::vertical + 2*directions::horizontal, files::bitboards::g | files::bitboards::h },
  };
  
  inline Bitboard pawn_attacks(Bitboard pawn, PawnDingbat const& pd, PawnAttackType const& pa) {
    return (pawn << (pd.leftshift + pa.leftshift) >> (pd.rightshift + pa.rightshift)) & ~pa.badtarget;
  }
  
  inline Bitboard bishop_attacks(squares::Index source, Bitboard occupancy) {
    Bitboard sources = squares::bitboard(source);
    return 
      slides(occupancy, sources, diagonals::bitboards::by_square(source) & ~sources) |
      slides(occupancy, sources, giadonals::bitboards::by_square(source) & ~sources);
  }
  
  inline Bitboard rook_attacks(squares::Index source, Bitboard occupancy) {
    Bitboard sources = squares::bitboard(source);
    return
      slides     (occupancy, sources, files::bitboards::by_square(source) & ~sources) |
      slides_rank(occupancy, sources, ranks::by_square(source));
  }
  
  inline Bitboard queen_attacks(squares::Index source, Bitboard occupancy) {
    return bishop_attacks(source, occupancy) | rook_attacks(source, occupancy);
  }
  
  inline Bitboard king_attacks(const Bitboard king) {
    using namespace files::bitboards;
    using namespace directions;
    Bitboard leftright = ((king & ~a) >> horizontal) | ((king & ~h) << horizontal);
    Bitboard triple = leftright | king;
    return leftright | (triple << vertical) | (triple >> vertical);
  }
  
  // NOTE: includes attacks on own pieces
  inline Bitboard attacks(Color us, Bitboard occupancy, Halfboard const& attackers) {
    Bitboard attacks = 0;
    
    for (const PawnAttackType &pa: pawn_attack_types)
      attacks |= pawn_attacks(attackers[pieces::pawn], pawn_dingbats[us], pa);
    
    for (const KnightAttackType &ka: knight_attack_types)
      attacks |= ka.attacks(attackers[pieces::knight]);
    
    squares::for_each(attackers[pieces::bishop], [&](squares::Index source) {
        attacks |= bishop_attacks(source, occupancy);
      });
    
    squares::for_each(attackers[pieces::rook], [&](squares::Index source) {
        attacks |= rook_attacks(source, occupancy);
      });
    
    squares::for_each(attackers[pieces::queen], [&](squares::Index source) {
        attacks |= queen_attacks(source, occupancy);
      });
    
    return attacks | king_attacks(attackers[pieces::king]);
  }

  inline Bitboard attackers(Bitboard targets, Bitboard occupancy, Color attacker, Halfboard const& attackers, bool early_return = false) {
    // put a superpiece on each target; if it attacks an attacker with the
    // appropriate mobility, the attacker is attacking at least one of the
    // targets.

    Color defender = colors::opposite(attacker);

    Bitboard sources = 0;

    for (const PawnAttackType &pa: pawn_attack_types) {
      sources |= pawn_attacks(targets, pawn_dingbats[defender], pa) & attackers[pieces::pawn];
      if (early_return && sources)
        return sources;
    }

    for (const KnightAttackType &ka: knight_attack_types) {
      sources |= ka.attacks(targets) & attackers[pieces::knight];
      if (early_return && sources)
        return sources;
    }

    auto fn = [&](squares::Index source) {
      Bitboard diagonal_attacks   = bishop_attacks(source, occupancy);
      Bitboard orthogonal_attacks = rook_attacks(source, occupancy);
      sources |= (diagonal_attacks   & (attackers[pieces::bishop] |
                                        attackers[pieces::queen]));
      sources |= (orthogonal_attacks & (attackers[pieces::rook] |
                                        attackers[pieces::queen]));
      return sources;
    };
    if (early_return) {
      if (squares::any(targets, fn))
        return true;
    } else {
      squares::for_each(targets, fn);
    }

    sources |= king_attacks(targets) & attackers[pieces::king];
    return sources;
  }

  inline bool any_attacked(Bitboard targets, Bitboard occupancy, Color attacker, Halfboard const& attackers) {
    return targets::attackers(targets, occupancy, attacker, attackers, true);
  }
}
