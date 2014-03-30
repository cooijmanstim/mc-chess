#pragma once

#include <vector>
#include <map>
#include <string>

class TrivialBoardPartition {
public:
  typedef size_t Index;

  // XXX: members should be considered const
  struct Part {
    Index index;
    std::string name;
    Bitboard bitboard;

    bool operator==(const Part& that) const { return this->bitboard == that.bitboard; }

    // bitwise operators on Parts imply bitboard operations
    Bitboard operator|(const Part& that) const { return this->bitboard | that.bitboard; }
    Bitboard operator&(const Part& that) const { return this->bitboard & that.bitboard; }
    Bitboard operator^(const Part& that) const { return this->bitboard ^ that.bitboard; }
    Bitboard operator~() const { return ~this->bitboard; }

    operator Bitboard() const { return this->bitboard; }
  };

  const size_t cardinality;
  const std::vector<Index> indices;
  const std::vector<Part> parts_by_index;
  const std::map<std::string, Part> parts_by_name;

  TrivialBoardPartition(std::initializer_list<std::string> names,
                        std::function<Bitboard(Index)> bitboard_from_index) :
    cardinality(names.size()),
    indices([&names]() {
        std::vector<Index> result;
        for (Index i = 0; i < names.size(); i++)
          result.push_back(i);
        return result;
      }()),
    parts_by_index([&names, &bitboard_from_index]() {
        std::vector<Part> result;
        Index i = 0; auto itname = names.begin();
        while (itname != names.end()) {
          result.emplace_back(Part{i, *itname, bitboard_from_index(i)});
          i++; itname++;
        }
        return result;
      }()),
    parts_by_name([this, &names]() {
        std::map<std::string, Part> result;
        Index i = 0; auto itname = names.begin();
        while (itname != names.end()) {
          result[*itname] = parts_by_index[i];
          i++; itname++;
        }
        return result;
      }())
  {
  }

  Part operator[](Index index)      const { return parts_by_index[index]; }
  Part operator[](std::string name) const { return parts_by_name.at(name); }
};
