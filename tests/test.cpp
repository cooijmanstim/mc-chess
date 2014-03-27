#define BOOST_TEST_MODULE test
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assign/list_of.hpp>

#include <algorithm>

#include "util.hpp"
#include "state.hpp"
#include "moves.hpp"

BOOST_AUTO_TEST_CASE(initial_moves) {
  State state;

  std::set<Move> expected_moves;
  using namespace directions;
  bitboard::for_each_member(ranks::_2, [&expected_moves](squares::Index from) {
      expected_moves.emplace(from, from +   north, Move::Type::normal);
      expected_moves.emplace(from, from + 2*north, Move::Type::double_push);
    });
  bitboard::for_each_member(squares::b1 | squares::g1, [&expected_moves](squares::Index from) {
      expected_moves.emplace(from, from + 2*north + west, Move::Type::normal);
      expected_moves.emplace(from, from + 2*north + east, Move::Type::normal);
    });

  const std::vector<Move> moves = state.moves();
  const std::set<Move> actual_moves(moves.begin(), moves.end());

  std::set<Move> falsenegatives, falsepositives;
  std::set_difference(expected_moves.begin(), expected_moves.end(),
                      actual_moves.begin(), actual_moves.end(),
                      std::inserter(falsenegatives, falsenegatives.begin()));
  std::set_difference(actual_moves.begin(), actual_moves.end(),
                      expected_moves.begin(), expected_moves.end(),
                      std::inserter(falsepositives, falsepositives.begin()));
  std::cout << "expected_moves: " << expected_moves << std::endl;
  std::cout << "actual_moves: " << actual_moves << std::endl;
  BOOST_CHECK_MESSAGE(falsenegatives.empty(), "legal moves not generated: " << falsenegatives);
  BOOST_CHECK_MESSAGE(falsepositives.empty(), "illegal moves generated: " << falsepositives);
}
