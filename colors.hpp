#pragma once

#include <cstddef>

namespace colors {
#define COLORS White, Black,
  enum Color { COLORS };
  const Color values[] = { COLORS };
#undef COLORS

  const std::size_t cardinality = sizeof(values) / sizeof(Color);
}

typedef enum colors::Color Color;
