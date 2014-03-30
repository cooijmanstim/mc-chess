#define BOOST_TEST_MODULE test
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assign/list_of.hpp>

#include <algorithm>

#include "util.hpp"
#include "direction.hpp"
#include "state.hpp"
#include "moves.hpp"

BOOST_AUTO_TEST_CASE(knight_attacks_on_border) {
  BOOST_CHECK_EQUAL(moves::knight_attacks_ssw(squares::b1), 0);
}

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
  BOOST_CHECK_MESSAGE(falsenegatives.empty(), "legal moves not generated: " << falsenegatives);
  BOOST_CHECK_MESSAGE(falsepositives.empty(), "illegal moves generated: " << falsepositives);
}

BOOST_AUTO_TEST_CASE(_moves) {
  State state;

  state.make_moves("e4 e5 Nf3 Nc6 Bc4 Bc5 b4 Bxb4 c3 Ba5 d4 exd4 0-0 d3 Qb3 Qf6"
                   "e5 Qg6 Re1 Nge7 Ba3 b5 Qxb5 Rb8 Qa4 Bb6 Nbd2 Bb7 Ne4 Qf5"
                   "Bxd3 Qh5 Nf6+ gxf6 18.exf6 Rg8 19.Rad1 Qxf3 20.Rxe7+ Nxe7"
                   "21.Qxd7+ Kxd7 22.Bf5+ Ke8 23.Bd7+ Kf8");

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
  BOOST_CHECK_MESSAGE(falsenegatives.empty(), "legal moves not generated: " << falsenegatives);
  BOOST_CHECK_MESSAGE(falsepositives.empty(), "illegal moves generated: " << falsepositives);
}
