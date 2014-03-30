#pragma once

#include <vector>
#include <map>
#include <string>

#include "bitboard.hpp"

class TrivialBoardPartition {
public:
  typedef size_t Index;

  // XXX: members should be considered const
  struct Part {
    Index index;
    std::string name;
    Bitboard bitboard;

    bool operator==(const Part& that) const;

    // bitwise operators on Parts imply bitboard operations
    Bitboard operator|(const Part& that) const;
    Bitboard operator&(const Part& that) const;
    Bitboard operator^(const Part& that) const;
    Bitboard operator~() const;

    operator Bitboard() const;
  };

  const size_t cardinality;
  const std::vector<Index> indices;
  const std::vector<Part> parts_by_index;
  const std::map<std::string, Part> parts_by_name;

  TrivialBoardPartition(std::initializer_list<std::string> names,
                        std::function<Bitboard(Index)> bitboard_from_index);

  Part operator[](Index index) const;
  Part operator[](std::string name) const;

  std::vector<Part>::const_iterator begin() const;
  std::vector<Part>::const_iterator end() const;
};

std::ostream& operator<<(std::ostream& o, const TrivialBoardPartition::Part& p);
