#include <cassert>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>

#include "boardpartition.hpp"
#include "partitions.hpp"

BoardPartition::BoardPartition(std::initializer_list<std::string> names,
                               std::function<Bitboard(Index)> bitboard_from_index) :
  TrivialBoardPartition(names, bitboard_from_index),
  parts_by_square_index([&names, this](){
      std::vector<Part> result;
      for (Part square: squares::partition) {
        bool found = false;
        for (Index i = 0; i < cardinality; i++) {
          if ((*this)[i].bitboard & square.bitboard) {
            if (found)
              throw std::logic_error(str(boost::format("square %1% is in multiple parts of partition %2%")
                                         % square % boost::algorithm::join(names, ", ")));
            result.push_back((*this)[i]);
            found = true;
          }
        }
        if (!found)
          throw std::logic_error(str(boost::format("square %1% is not in partition %2%")
                                     % square % boost::algorithm::join(names, ", ")));
      }
      return result;
    }())
{
}
