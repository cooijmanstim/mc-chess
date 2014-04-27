/* Okay, so this is highly objectionable, but it saves a huge amount of work.
 * Provide appropriate #defineitions for PARTITION_*, #include this and voila,
 * an enum with indices, an array for name lookup, and mappings between indices
 * and bitboards.
 */
#if !defined(PARTITION_CARDINALITY) || !defined(PARTITION_KEYWORDS) || !defined(PARTITION_BITBOARD)
#error "not all template variables defined"
#endif

  const size_t cardinality = PARTITION_CARDINALITY;

#define _(key) key,
  enum Index {
    PARTITION_KEYWORDS
  };
  std::array<Index, cardinality> indices = {
    PARTITION_KEYWORDS
  };
#undef _

#define _(key) #key,
  std::array<std::string, cardinality> names = {
    PARTITION_KEYWORDS
  };
#undef _

  inline const Bitboard& bitboard(Index i) {
    static auto bitboards = [](){
      std::array<Bitboard, cardinality> bitboards;
      for (Index i: indices)
        bitboards[i] = PARTITION_BITBOARD(i);
      return bitboards;
    }();
    return bitboards[i];
  }

#undef PARTITION_CARDINALITY
#undef PARTITION_KEYWORDS
#undef PARTITION_BITBOARD
