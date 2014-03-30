#pragma once

#include "trivialboardpartition.hpp"

#include <vector>

class BoardPartition : public TrivialBoardPartition {
public:
  const std::vector<Part> parts_by_square_index;

  BoardPartition(std::initializer_list<std::string> names,
                 std::function<Bitboard(Index)> bitboard_from_index);
};
