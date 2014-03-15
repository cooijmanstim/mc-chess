#pragma once

#include <string>
#include <cstddef>

namespace colors {
#define COLORS white, black,
  enum Color { COLORS };
  const Color values[] = { COLORS };
#undef COLORS

  const size_t cardinality = sizeof(values) / sizeof(Color);

  std::string name(Color color);
}

typedef enum colors::Color Color;
