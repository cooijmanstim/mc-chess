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

  inline const std::vector<PawnDingbat>& get_pawn_dingbats() {
    using namespace directions;
    static std::vector<PawnDingbat> pawn_dingbats = {
      { vertical, 0, ranks::bitboards::_4, ranks::bitboards::_8 },
      { 0, vertical, ranks::bitboards::_5, ranks::bitboards::_1 },
    };
    return pawn_dingbats;
  }
  
  inline const std::vector<PawnAttackType>& get_pawn_attack_types() {
    using namespace files::bitboards;
    using namespace directions;
    static std::vector<PawnAttackType> result = {
      { 0, horizontal, h },
      { horizontal, 0, a },
    };
    return result;
  }
  
  inline const std::vector<KnightAttackType>& get_knight_attack_types() {
    using namespace files::bitboards;
    using namespace directions;
    static std::vector<KnightAttackType> result = {
      {2*vertical +   horizontal,                         0, a     },
      {               horizontal, 2*vertical               , a     },
      {2*vertical               ,                horizontal, h     },
      {                        0, 2*vertical +   horizontal, h     },
      {  vertical + 2*horizontal,                         0, a | b },
      {             2*horizontal,   vertical               , a | b },
      {  vertical               ,              2*horizontal, g | h },
      {                        0,   vertical + 2*horizontal, g | h },
    };
    return result;
  }
  
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
    
    for (PawnAttackType pa: get_pawn_attack_types())
      attacks |= pawn_attacks(attackers[pieces::pawn], get_pawn_dingbats()[us], pa);
    
    for (KnightAttackType ka: get_knight_attack_types())
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
}
