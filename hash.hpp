#pragma once

#include <cstdint>

#include <boost/random.hpp>

#include "util.hpp"
#include "colors.hpp"
#include "pieces.hpp"
#include "bitboard.hpp"
#include "partitions.hpp"
#include "castles.hpp"

typedef uint64_t Hash;

namespace hashes {
  struct Hashes {
    Hash black_to_move;
    array3d<Hash, squares::cardinality, colors::cardinality, pieces::cardinality> colored_piece_at_square;
    array2d<Hash, colors::cardinality, castles::cardinality> can_castle;
    std::array<Hash, files::cardinality> en_passant;
  };

  inline Hash generate_random_feature() {
    static boost::random::mt19937 base_generator;
    static boost::random::uniform_int_distribution<Hash> distribution;
    static boost::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<Hash>> generator(base_generator, distribution);
    return generator();
  }

  inline const Hashes& get_hashes() {
    const static Hashes hashes = {
      generate_random_feature(),
      []() {
        array3d<Hash, squares::cardinality, colors::cardinality, pieces::cardinality> x;
        for (squares::Index s: squares::indices)
          for (Color c: colors::values)
            for (Piece p: pieces::values)
              x[s][c][p] = generate_random_feature();
        return x;
      }(),
      []() {
        array2d<Hash, colors::cardinality, castles::cardinality> x;
        for (Color color: colors::values)
          for (Castle castle: castles::values)
            x[color][castle] = generate_random_feature();
        return x;
      }(),
      []() {
        std::array<Hash, files::cardinality> x;
        for (files::Index file: files::indices)
          x[file] = generate_random_feature();
        return x;
      }(),
    };
    return hashes;
  }

  inline Hash black_to_move() {
    return get_hashes().black_to_move;
  }
  
  inline Hash colored_piece_at_square(const Color color, Piece piece, squares::Index square) {
    return get_hashes().colored_piece_at_square[square][color][piece];
  }
  
  inline Hash can_castle(Color color, Castle castle) {
    return get_hashes().can_castle[color][castle];
  }
  
  inline Hash en_passant(Bitboard en_passant_square) {
    files::Index file = files::by_square(squares::index(en_passant_square));
    return get_hashes().en_passant[file];
  }
  
  inline void toggle(Hash& hash, Color color, Piece piece, squares::Index square) {
    hash ^= colored_piece_at_square(color, piece, square);
  }
}
