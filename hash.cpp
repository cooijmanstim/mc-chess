#include "hash.hpp"

Hash hashes::generate_random_feature() {
  static boost::random::mt19937 base_generator;
  static boost::random::uniform_int_distribution<Hash> distribution;
  static boost::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<Hash>> generator(base_generator, distribution);
  return generator();
}

Hash hashes::black_to_move() {
  static Hash x = generate_random_feature();
  return x;
}

Hash hashes::colored_piece_at_square(const Color color, const Piece piece, const squares::Index square) {
  static auto x = []() {
    array3d<Hash, squares::cardinality, colors::cardinality, pieces::cardinality> x;
    for (Square s: squares::partition)
      for (Color c: colors::values)
        for (Piece p: pieces::values)
          x[s.index][c][p] = generate_random_feature();
    return x;
  }();
  return x[square][color][piece];
}

Hash hashes::can_castle(const Color color, const Castle castle) {
  static auto x = []() {
    array2d<Hash, colors::cardinality, Castle::Type::cardinality> x;
    for (Color color: colors::values)
      for (Castle castle: castles::values)
        x[color][castle] = generate_random_feature();
    return x;
  }();
  return x[color][castle];
}

Hash hashes::en_passant(const Bitboard en_passant_square) {
  static auto x = []() {
    std::array<Hash, files::cardinality> x;
    for (File file: files::partition)
      x[file.index] = generate_random_feature();
    return x;
  }();
  File file = files::partition.parts_by_square_index[squares::index_from_bitboard(en_passant_square)];
  return x[file.index];
}

void hashes::toggle(Hash& hash, const Color color, const Piece piece, const squares::Index square) {
  hash ^= colored_piece_at_square(color, piece, square);
}
