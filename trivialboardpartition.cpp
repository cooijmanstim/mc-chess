#include "trivialboardpartition.hpp"

bool TrivialBoardPartition::Part::operator==(const Part& that) const { return this->bitboard == that.bitboard; }

Bitboard TrivialBoardPartition::Part::operator|(const Part& that) const { return this->bitboard | that.bitboard; }
Bitboard TrivialBoardPartition::Part::operator&(const Part& that) const { return this->bitboard & that.bitboard; }
Bitboard TrivialBoardPartition::Part::operator^(const Part& that) const { return this->bitboard ^ that.bitboard; }
Bitboard TrivialBoardPartition::Part::operator~() const { return ~this->bitboard; }

TrivialBoardPartition::Part::operator Bitboard() const { return this->bitboard; }

TrivialBoardPartition::TrivialBoardPartition(std::initializer_list<std::string> names,
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

TrivialBoardPartition::Part TrivialBoardPartition::operator[](Index index)      const { return parts_by_index[index]; }
TrivialBoardPartition::Part TrivialBoardPartition::operator[](std::string name) const { return parts_by_name.at(name); }
