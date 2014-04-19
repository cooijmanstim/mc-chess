#include <cstdint>

#include <boost/random.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "pieces.hpp"
#include "bitboard.hpp"
#include "partitions.hpp"

typedef uint64_t Hash;

namespace hashes {
  Hash generate_random_feature();

  Hash black_to_move();
  Hash colored_piece_at_square(const Color color, const Piece piece, const squares::Index square);
  Hash can_castle_kingside(const Color color);
  Hash can_castle_queenside(const Color color);
  Hash en_passant(const Bitboard en_passant_square);

  void toggle(Hash& hash, const Color color, const Piece piece, const squares::Index square);
}
