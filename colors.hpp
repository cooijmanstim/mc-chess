#pragma once

#include <cstddef>

namespace colors {
#define COLORS white, black,
  enum Color { COLORS };
  const Color values[] = { COLORS };
#undef COLORS

  const size_t cardinality = sizeof(values) / sizeof(Color);
}

typedef enum colors::Color Color;
