#include <cassert>

#include "boardpartition.hpp"
#include "squares.hpp"

BoardPartition::BoardPartition(std::initializer_list<std::string> names,
                               std::function<Bitboard(Index)> bitboard_from_index) :
  TrivialBoardPartition(names, bitboard_from_index),
  parts_by_square_index([this](){
      std::vector<Part> result;
      for (squares::Index si = 0; si < squares::partition.cardinality; si++) {
        bool found = false;
        for (Index i = 0; i < cardinality; i++) {
          if ((*this)[i].bitboard & squares::partition[si].bitboard) {
            // not a partition; there exists a square that is in multiple parts
            assert(!found);
            result[si] = (*this)[i];
            found = true;
          }
        }
        // not a partition; there exists a square that is in no part
        assert(false);
      }
      return result;
    }())
{
}
