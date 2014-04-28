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
    for (squares::Index s: squares::indices)
      for (Color c: colors::values)
        for (Piece p: pieces::values)
          x[s][c][p] = generate_random_feature();
    return x;
  }();
  return x[square][color][piece];
}

Hash hashes::can_castle(const Color color, const Castle castle) {
  static auto x = []() {
    array2d<Hash, colors::cardinality, castles::cardinality> x;
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
    for (files::Index file: files::indices)
      x[file] = generate_random_feature();
    return x;
  }();
  files::Index file = files::by_square(squares::index(en_passant_square));
  return x[file];
}

void hashes::toggle(Hash& hash, const Color color, const Piece piece, const squares::Index square) {
  hash ^= colored_piece_at_square(color, piece, square);
}
