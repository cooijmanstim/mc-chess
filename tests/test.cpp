#define BOOST_TEST_MODULE test
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assign/list_of.hpp>

#include <algorithm>

#include "state.hpp"
#include "moves.hpp"

BOOST_AUTO_TEST_CASE(initial_moves) {
  State state;

  std::set<Move> expected_moves;
  bitboard::for_each_member(ranks::_2, [expected_moves](squares::Index from) {
      expected_moves.add(Move(from, from +   north, Move::types::normal));
      expected_moves.add(Move(from, from + 2*north, Move::types::double_push));
    });
  bitboard::for_each_member(squares::b1 || squares::g1, [expected_moves](squares::Index from) {
      expected_moves.add(Move(from, from + 2*north + west, Move::types::normal));
      expected_moves.add(Move(from, from + 2*north + east, Move::types::normal));
    });

  std::set<Move> actual_moves(state.moves());

  std::set<Move> falsenegatives, falsepositives;
  std::set_difference(expected_moves.begin(), expected_moves.end(),
                      actual_moves.begin(), actual_moves.begin(),
                      falsenegatives);
  std::set_difference(actual_moves.begin(), actual_moves.end(),
                      expected_moves.begin(), expected_moves.end(),
                      falsepositives);
  BOOST_CHECK_MESSAGE(falsenegatives.empty(), "illegal moves generated: " << falsenegatives);
  BOOST_CHECK_MESSAGE(falsepositives.empty(), "legal moves not generated: " << falsepositives);
}
