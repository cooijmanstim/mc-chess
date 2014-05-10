#include <iostream>
#include <algorithm>

#define BOOST_TEST_MODULE test
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include "util.hpp"
#include "direction.hpp"
#include "state.hpp"
#include "move_generation.hpp"
#include "hash.hpp"
#include "mcts_agent.hpp"

// NOTE: evaluates arguments twice
#define BOOST_CHECK_BITBOARDS_EQUAL(a, b) \
  BOOST_CHECK_MESSAGE((a) == (b), boost::format("%|1$#x| != %|2$#x|") % (a) % (b));

BOOST_AUTO_TEST_CASE(randompoop) {
  hashes::generate_random_feature();
}

BOOST_AUTO_TEST_CASE(partitions) {
  using namespace squares::bitboards;
  BOOST_CHECK_EQUAL(bitboard::cardinality(0x1), 1);
  BOOST_CHECK_EQUAL(bitboard::cardinality(0x400000), 1);
  BOOST_CHECK_BITBOARDS_EQUAL(a1, 0x1);
  BOOST_CHECK_BITBOARDS_EQUAL(h8, 0x8000000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(e4, 0x0000000010000000);
  BOOST_CHECK_BITBOARDS_EQUAL(d4 | e4 | f4 | c5, 0x0000000438000000);
  BOOST_CHECK_BITBOARDS_EQUAL(files::bitboards::a,  a1 | a2 | a3 | a4 | a5 | a6 | a7 | a8);
  BOOST_CHECK_BITBOARDS_EQUAL(ranks::bitboards::_8, a8 | b8 | c8 | d8 | e8 | f8 | g8 | h8);
  BOOST_CHECK_BITBOARDS_EQUAL(giadonals::bitboards::a1h8, a1 | b2 | c3 | d4 | e5 | f6 | g7 | h8);
  BOOST_CHECK_BITBOARDS_EQUAL(diagonals::bitboards::a8h1, a8 | b7 | c6 | d5 | e4 | f3 | g2 | h1);
}

BOOST_AUTO_TEST_CASE(initial_moves) {
  State state;

  std::cout << state << std::endl;

  std::set<Move> expected_moves;
  using namespace directions;
  squares::do_bits(ranks::bitboards::_2, [&expected_moves](squares::Index from) {
      expected_moves.emplace(from, from +   north, move_types::normal);
      expected_moves.emplace(from, from + 2*north, move_types::double_push);
    });
  squares::do_bits(squares::bitboards::b1 | squares::bitboards::g1, [&expected_moves](squares::Index from) {
      expected_moves.emplace(from, from + 2*north + west, move_types::normal);
      expected_moves.emplace(from, from + 2*north + east, move_types::normal);
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

BOOST_AUTO_TEST_CASE(rays_every_which_way) {
  squares::Index bishop_square = squares::f5, rook_square = squares::c3; 

  Bitboard diagonal = diagonals::bitboards::by_square(bishop_square);
  Bitboard giadonal = giadonals::bitboards::by_square(bishop_square);
  ranks::Index rank = ranks::by_square(rook_square);
  Bitboard file = files::bitboards::by_square(rook_square);

  Bitboard bishop_board = squares::bitboard(bishop_square);
  Bitboard rook_board = squares::bitboard(rook_square);

  Bitboard bda = moves::slides(bishop_board, bishop_board, diagonal & ~bishop_board);
  Bitboard bga = moves::slides(bishop_board, bishop_board, giadonal & ~bishop_board);
  Bitboard rra = moves::slides_rank(rook_board, rook_board, rank);
  Bitboard rfa = moves::slides(rook_board, rook_board, file & ~rook_board);

  BOOST_CHECK_BITBOARDS_EQUAL(bda, 0x0408100040800000);
  BOOST_CHECK_BITBOARDS_EQUAL(bga, 0x0080400010080402);
  BOOST_CHECK_BITBOARDS_EQUAL(rra, 0x0000000000fb0000);
  BOOST_CHECK_BITBOARDS_EQUAL(rfa, 0x0404040404000404);

  State state("8/8/8/5B2/8/2R5/8/8 w - - 0 1");
  Bitboard flat_occupancy;
  board::flatten(state.occupancy, flat_occupancy);
  BOOST_CHECK_BITBOARDS_EQUAL(moves::bishop_attacks(flat_occupancy, bishop_square),
                              0x0488500050880402);
  BOOST_CHECK_BITBOARDS_EQUAL(moves::rook_attacks(flat_occupancy, rook_square),
                              0x0404040404fb0404);
}

BOOST_AUTO_TEST_CASE(various_moves) {
  State state("r1b2rk1/pp1P1p1p/q1p2n2/2N2PpB/1NP2bP1/2R1B3/PP2Q2P/R3K3 w Q g6 0 1");

  {
    using namespace colors;
    using namespace pieces;
    using namespace castles;
    using namespace squares::bitboards;
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][pawn],   a2 | b2 | c4 | d7 | f5 | g4 | h2);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][knight], b4 | c5);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][bishop], e3 | h5);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][rook],   a1 | c3);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][queen],  e2);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][king],   e1);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][pawn],   a7 | b7 | c6 | f7 | g5 | h7);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][knight], f6);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][bishop], c8 | f4);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][rook],   a8 | f8);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][queen],  a6);
    BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][king],   g8);
    BOOST_CHECK_BITBOARDS_EQUAL(state.en_passant_square,    g6);
    BOOST_CHECK_BITBOARDS_EQUAL(state.their_attacks, 0xfeef5fdbf5518100);
    BOOST_CHECK(!state.castling_rights[white][kingside]);
    BOOST_CHECK(!state.castling_rights[black][kingside]);
    BOOST_CHECK( state.castling_rights[white][queenside]);
    BOOST_CHECK(!state.castling_rights[black][queenside]);
    BOOST_CHECK_EQUAL(state.us, white);
  }

  std::cout << state << std::endl;

  Bitboard flat_occupancy;
  board::flatten(state.occupancy, flat_occupancy);
  BOOST_CHECK_BITBOARDS_EQUAL(moves::rook_attacks(flat_occupancy, squares::c3),
                              0x00000000041b0404);

  BOOST_CHECK_BITBOARDS_EQUAL(moves::king_attacks(state.board[colors::white][pieces::king]),
                              0x0000000000003828);

  std::set<Move> expected_moves;

  {
    using namespace squares;
    // a1 rook
    for (squares::Index target: {b1, c1, d1})
      expected_moves.emplace(Move(a1, target, move_types::normal));
  
    // e1 king
    for (squares::Index target: {d1, f1, d2, f2})
      expected_moves.emplace(Move(e1, target, move_types::normal));
    expected_moves.emplace(Move(e1, c1, move_types::castle_queenside));
  
    // a2 pawn
    expected_moves.emplace(Move(a2, a3, move_types::normal));
    expected_moves.emplace(Move(a2, a4, move_types::double_push));
  
    // b2 pawn
    expected_moves.emplace(Move(b2, b3, move_types::normal));
  
    // e2 queen
    for (squares::Index target: {f1, f2, g2, f3, d3, d2, c2, d1})
      expected_moves.emplace(Move(e2, target, move_types::normal));
  
    // h2 pawn
    expected_moves.emplace(Move(h2, h3, move_types::normal));
    expected_moves.emplace(Move(h2, h4, move_types::double_push));
  
    // c3 rook
    for (squares::Index target: {c2, c1, d3, b3, a3})
      expected_moves.emplace(Move(c3, target, move_types::normal));
    
    // e3 bishop
    for (squares::Index target: {f2, g1, d4, d2, c1})
      expected_moves.emplace(Move(e3, target, move_types::normal));
    expected_moves.emplace(Move(e3, f4, move_types::capture));
  
    // b4 knight
    for (squares::Index target: {d5, d3, c2})
      expected_moves.emplace(Move(b4, target, move_types::normal));
    for (squares::Index target: {a6, c6})
      expected_moves.emplace(Move(b4, target, move_types::capture));
  
    // c4 pawn
  
    // g4 pawn
  
    // c5 knight
    for (squares::Index target: {e6, e4, d3, b3, a4})
      expected_moves.emplace(Move(c5, target, move_types::normal));
    for (squares::Index target: {a6, b7})
      expected_moves.emplace(Move(c5, target, move_types::capture));
  
    // f5 pawn
    expected_moves.emplace(Move(f5, g6, move_types::capture));
  
    // h5 bishop
    expected_moves.emplace(Move(h5, g6, move_types::normal));
    expected_moves.emplace(Move(h5, f7, move_types::capture));
  
    // d7 pawn
    for (MoveType type: {move_types::promotion_knight,
                         move_types::promotion_bishop,
                         move_types::promotion_rook,
                         move_types::promotion_queen}) {
      expected_moves.emplace(Move(d7, d8, type));
    }
    for (MoveType type: {move_types::capturing_promotion_knight,
                         move_types::capturing_promotion_bishop,
                         move_types::capturing_promotion_rook,
                         move_types::capturing_promotion_queen}) {
      expected_moves.emplace(Move(d7, c8, type));
    }
  }

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

BOOST_AUTO_TEST_CASE(algebraic_moves) {
  using namespace colors;
  using namespace pieces;
  using namespace squares;

  State state;

  BOOST_CHECK(Move(e2, e4, move_types::double_push).matches_algebraic(NULL, NULL, e4, false, NULL));

  state.make_moves("e4 e5 Nf3 Nc6 Bc4 Bc5 b4 Bxb4 c3 Ba5 d4 exd4 0-0 d3 Qb3 Qf6");

  BOOST_CHECK_BITBOARDS_EQUAL(state.occupancy[white], 0x000000001426e167);
  BOOST_CHECK_BITBOARDS_EQUAL(state.occupancy[black], 0xd5ef240100080000);

  state.make_moves("e5 Qg6 Re1 Nge7 Ba3 b5 Qxb5 Rb8 Qa4 Bb6 Nbd2 Bb7 Ne4 Qf5 "
                   "Bxd3 Qh5 Nf6+ gxf6 exf6 Rg8 Rad1 Qxf3 Rxe7+ Nxe7 Qxd7+ "
                   "Kxd7 Bf5+ Ke8 Bd7+ Kf8");

  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][pawn],   0x000020000004e100);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][knight], 0x0000000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][bishop], 0x0008000000010000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][rook],   0x0000000000000008);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][queen],  0x0000000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[white][king],   0x0000000000000040);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][pawn],   0x00a5000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][knight], 0x0010000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][bishop], 0x0002020000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][rook],   0x4200000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][queen],  0x0000000000200000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.board[black][king],   0x2000000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.en_passant_square,    0x0000000000000000);
  BOOST_CHECK_BITBOARDS_EQUAL(state.their_attacks,        0xfd777fed78fc7008);
  BOOST_CHECK_BITBOARDS_EQUAL(state.occupancy[white],     0x000820000005e148);
  BOOST_CHECK_BITBOARDS_EQUAL(state.occupancy[black],     0x62b7020000200000);
  BOOST_CHECK_EQUAL(state.us, white);

  std::set<Move> expected_moves;
#define MV(from, to, type) expected_moves.emplace(from, to, type);
  MV(c3, c4, move_types::normal);
  MV(g2, g3, move_types::normal);
  MV(g2, g4, move_types::double_push);
  MV(h2, h3, move_types::normal);
  MV(h2, h4, move_types::double_push);
  for (squares::Index target: {a4, b5, c6, e8, c8, e6, f5, g4, h3})
    MV(d7, target, move_types::normal);
  for (squares::Index target: {c1, b2, b4, c5, d6})
    MV(a3, target, move_types::normal);
  MV(a3, e7, move_types::capture);
  for (squares::Index target: {a1, b1, c1, e1, f1, d2, d3, d4, d5, d6})
    MV(d1, target, move_types::normal);
  MV(f6, e7, move_types::capture);
  MV(g1, f1, move_types::normal);
  MV(g1, h1, move_types::normal);
#undef MV

  std::cout << state << std::endl;

  std::vector<Move> moves = state.moves();
  std::set<Move> actual_moves(moves.begin(), moves.end());

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

BOOST_AUTO_TEST_CASE(move_randomly) {
  State state;
  boost::mt19937 generator;
  for (int i = 0; i < 100; i++) {
    boost::optional<Move> move = state.random_move(generator);
    if (!move)
      break;
    state.make_move(*move);
    Hash hash;
    state.compute_hash(hash);
    BOOST_CHECK(hash != 0); // probably not zero
    BOOST_CHECK_EQUAL(state.hash, hash);
  }
}

BOOST_AUTO_TEST_CASE(mcts) {
  State state;
  MCTSAgent agent;
  auto decision = agent.start_decision(state);
  decision.get();
}
