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
  char symbol(Color color);

  inline Color opposite(Color color) {
    return static_cast<Color>(1 - color);
  }
}

typedef enum colors::Color Color;
