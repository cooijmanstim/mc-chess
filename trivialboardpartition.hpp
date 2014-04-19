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
    bool operator!=(const Part& that) const;
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

#define pfft(op) \
Bitboard operator op (const Bitboard x, const TrivialBoardPartition::Part& y); \
Bitboard operator op (const TrivialBoardPartition::Part& y, const Bitboard x); \
Bitboard operator op (const TrivialBoardPartition::Part& x, const TrivialBoardPartition::Part& y); \
Bitboard operator op ## = (Bitboard& x, const TrivialBoardPartition::Part& y);
pfft(&);
pfft(|);
pfft(^);
#undef pfft

bool operator==(const TrivialBoardPartition::Part& y, const Bitboard x);
bool operator==(const Bitboard x, const TrivialBoardPartition::Part& y);

Bitboard operator~(const TrivialBoardPartition::Part& x);
Bitboard operator<<(const TrivialBoardPartition::Part& x, const unsigned int k);
Bitboard operator>>(const TrivialBoardPartition::Part& x, const unsigned int k);

