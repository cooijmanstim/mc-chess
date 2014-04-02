#include "knight.hpp"
#include "direction.hpp"
#include "partitions.hpp"

std::vector<KnightAttackType> get_knight_attack_types() {
  using namespace directions;
  using namespace files;
  static std::vector<KnightAttackType> result = {
    {2*vertical +   horizontal,                         0, a    },
    {               horizontal, 2*vertical               , a    },
    {2*vertical               ,                horizontal, h    },
    {                        0, 2*vertical +   horizontal, h    },
    {  vertical + 2*horizontal,                         0, a | b},
    {             2*horizontal,   vertical               , a | b},
    {  vertical               ,              2*horizontal, g | h},
    {                        0,   vertical + 2*horizontal, g | h},
  };
  return result;
}
