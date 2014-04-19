#include <boost/format.hpp>

#include "prettyprint.hpp"
#include "trivialboardpartition.hpp"

#define pfft(op) \
Bitboard operator op (const Bitboard x, const TrivialBoardPartition::Part& y) { return x op y.bitboard; } \
Bitboard operator op (const TrivialBoardPartition::Part& y, const Bitboard x) { return y.bitboard op x; } \
Bitboard operator op (const TrivialBoardPartition::Part& x, const TrivialBoardPartition::Part& y) { return x.bitboard op y.bitboard; } \
Bitboard operator op ## = (Bitboard& x, const TrivialBoardPartition::Part& y) { return x op ## = y.bitboard; }
pfft(&);
pfft(|);
pfft(^);
#undef pfft
Bitboard operator~(const TrivialBoardPartition::Part& x) { return ~x.bitboard; }
Bitboard operator<<(const TrivialBoardPartition::Part& x, const unsigned int k) { return x.bitboard << k; }
Bitboard operator>>(const TrivialBoardPartition::Part& x, const unsigned int k) { return x.bitboard >> k; }

bool operator==(const TrivialBoardPartition::Part& y, const Bitboard x) { return y.bitboard == x; }
bool operator==(const Bitboard x, const TrivialBoardPartition::Part& y) { return y.bitboard == x; }

bool TrivialBoardPartition::Part::operator==(const TrivialBoardPartition::Part& that) const { return this->bitboard == that.bitboard; }
bool TrivialBoardPartition::Part::operator!=(const TrivialBoardPartition::Part& that) const { return this->bitboard != that.bitboard; }

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

TrivialBoardPartition::Part TrivialBoardPartition::operator[](Index index)      const {
  assert(index < parts_by_index.size());
  return parts_by_index[index];
}
TrivialBoardPartition::Part TrivialBoardPartition::operator[](std::string name) const {
  try {
    return parts_by_name.at(name);
  } catch (std::out_of_range& e) {
    throw std::invalid_argument(str(boost::format("unknown part name: %1% in partition %2%") % name % parts_by_index));
  }
}

std::vector<TrivialBoardPartition::Part>::const_iterator TrivialBoardPartition::begin() const { return parts_by_index.begin(); }
std::vector<TrivialBoardPartition::Part>::const_iterator TrivialBoardPartition::end()   const { return parts_by_index.end(); }

std::ostream& operator<<(std::ostream& o, const TrivialBoardPartition::Part& p) {
  o << boost::format("Part(index=%1%, name=%2%, bitboard=%|3$#x|)") % p.index % p.name % p.bitboard;
  return o;
}
