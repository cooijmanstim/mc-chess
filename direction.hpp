#pragma once

typedef int Direction;

namespace directions {
  const Direction south = -8, west = -1, east = 1, north = 8;

  /* These are used to avoid negative shift counts:
   *   "<< north" == "<< vertical"
   *   "<< south" == ">> north" == ">> vertical"
   * Less descriptive but at least there's not the deceptive north/south.
   *
   * << horizontal is eastward, >> horizontal is westward,
   * << vertical is northward, >> vertical is southward
   */
  const Direction horizontal = 1, vertical = 8;
}
