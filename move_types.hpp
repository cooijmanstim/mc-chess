#include <string>
#include <vector>

namespace move_types {
  // NOTE: at most 16!
#define KEYWORDS  \
    _(normal) \
    _(double_push) \
    _(castle_kingside) \
    _(castle_queenside) \
    _(capture) \
    _(promotion_knight) \
    _(promotion_bishop) \
    _(promotion_rook) \
    _(promotion_queen) \
    _(capturing_promotion_knight) \
    _(capturing_promotion_bishop) \
    _(capturing_promotion_rook) \
    _(capturing_promotion_queen)

#define _(key) key,
  enum Type {
    KEYWORDS
  };
#undef _

#define _(key) #key,
  static const std::vector<std::string> keywords = {
    KEYWORDS
  };
#undef _
}

typedef move_types::Type MoveType;
