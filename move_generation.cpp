#include <stdexcept>
#include <cassert>

#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include "move_generation.hpp"
#include "direction.hpp"
#include "targets.hpp"
#include "state.hpp"

// helper to generate normal or promoting moves for pawns given source and target.
void maybe_promoting(std::vector<Move>& moves, State const& state, squares::Index source, squares::Index target, MoveType tentative_type) {
  if (squares::bitboard(target) & targets::pawn_dingbats[state.us].promotion_rank) {
    using namespace move_types;
    if (tentative_type == capture) {
      for (MoveType type: {capturing_promotion_knight, capturing_promotion_bishop,
                           capturing_promotion_rook,   capturing_promotion_queen})
        moves.emplace_back(source, target, type);
    } else {
      for (MoveType type: {promotion_knight, promotion_bishop,
                           promotion_rook,   promotion_queen})
        moves.emplace_back(source, target, type);
    }
  } else {
    moves.emplace_back(source, target, tentative_type);
  }
}

// generate normal or capturing moves based on whether targets are occupied by
// our or their pieces.  not valid for pawns because their normal movement is
// different from their capture movement.
template <typename F>
void maybe_capturing(std::vector<Move>& moves, State const& state, F source_fn, Bitboard targets) {
  squares::for_each(targets & ~state.flat_occupancy, [&](squares::Index target) {
      moves.emplace_back(source_fn(target), target, move_types::normal);
    });
  squares::for_each(targets & state.occupancy[state.them], [&](squares::Index target) {
      moves.emplace_back(source_fn(target), target, move_types::capture);
    });
}

// helper to generate sliding piece moves.
template <typename F>
void slider_moves(std::vector<Move>& moves, State const& state, Bitboard sources, F attack_fn) {
  squares::for_each(sources, [&](squares::Index source) {
    maybe_capturing(moves, state,
                    [&](squares::Index target) { return source; },
                    attack_fn(source, state.flat_occupancy));
  });
}

void moves::king_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  squares::Index source = squares::index(sources);
  maybe_capturing(moves, state,
                  [&](squares::Index target) { return source; },
                  targets::king_attacks(sources) & ~state.their_attacks);
}

void moves::queen_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  return slider_moves(moves, state, sources, targets::queen_attacks);
}

void moves::rook_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  return slider_moves(moves, state, sources, targets::rook_attacks);
}

void moves::bishop_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  return slider_moves(moves, state, sources, targets::bishop_attacks);
}

void moves::knight_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  for (targets::KnightAttackType const& ka: targets::knight_attack_types) {
    maybe_capturing(moves, state,
                    [&](squares::Index target) {
                      return static_cast<squares::Index>(target - ka.direction());
                    },
                    ka.attacks(sources));
  }
}

void moves::pawn_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  const targets::PawnDingbat &pd = targets::pawn_dingbats[state.us];

  // single push
  squares::for_each(pd.single_push_targets(sources, state.flat_occupancy), [&](squares::Index target) {
      squares::Index source = static_cast<squares::Index>(target - pd.single_push_direction());
      maybe_promoting(moves, state, source, target, move_types::normal);
    });

  // double push
  squares::for_each(pd.double_push_targets(sources, state.flat_occupancy), [&](squares::Index target) {
      squares::Index source = static_cast<squares::Index>(target - pd.double_push_direction());
      moves.emplace_back(source, target, move_types::double_push);
    });

  // captures
  for (const targets::PawnAttackType& pa: targets::pawn_attack_types) {
    signed direction = pd.leftshift - pd.rightshift + pa.leftshift - pa.rightshift;
    squares::for_each(pawn_attacks(sources, pd, pa) & (state.occupancy[state.them] | state.en_passant_square),
                      [&](squares::Index target) {
                        maybe_promoting(moves, state,
                                        static_cast<squares::Index>(target - direction),
                                        target, move_types::capture);
                      });
  }
}

void moves::castle_moves(std::vector<Move>& moves, State const& state) {
  for (Castle castle: castles::values) {
    if (state.can_castle(castle))
      moves.push_back(Move::castle(state.us, castle));
  }
}

// generate moves capturing the target, except from badsources
void moves::capturing(std::vector<Move>& moves, State const& state, squares::Index target, bool to_block_check) {
  Bitboard targets = squares::bitboard(target);

  Bitboard sources = targets::attackers(targets, state.flat_occupancy, state.us, state.board[state.us]);
  sources &= ~state.board[state.us][pieces::pawn];
  if (to_block_check)
    sources &= ~state.board[state.us][pieces::king];
  squares::for_each(sources, [&](squares::Index source) {
      moves.emplace_back(source, target, move_types::capture);
    });

  pawn_moves_capturing(moves, state, target);
}

// generate moves that occupy the target
// if to_block_check is true, king moves (and castles) will not be included
// NOTE: target assumed vacant
void moves::occupying(std::vector<Move>& moves, State const& state, squares::Index target, bool to_block_check) {
  Bitboard sources = targets::attackers(squares::bitboard(target), state.flat_occupancy, state.us, state.board[state.us]);
  sources &= ~state.board[state.us][pieces::pawn];
  if (to_block_check)
    sources &= ~state.board[state.us][pieces::king];
  squares::for_each(sources, [&](squares::Index source) {
      moves.emplace_back(source, target, move_types::normal);
    });

  moves::pawn_moves_occupying(moves, state, target);

  if (!to_block_check) {
    for (Castle castle: castles::values) {
      if ((castles::king_target(state.us, castle) == target ||
           castles::rook_target(state.us, castle) == target) &&
          state.can_castle(castle))
        moves.push_back(Move::castle(state.us, castle));
    }
  }
}

// NOTE: target assumed vacant
void moves::pawn_moves_occupying(std::vector<Move>& moves, State const& state, squares::Index target) {
  Bitboard targets = squares::bitboard(target);

  const targets::PawnDingbat &pd = targets::pawn_dingbats[state.us];

  if (targets & pd.single_push_targets(state.board[state.us][pieces::pawn], state.flat_occupancy)) {
    squares::Index source = static_cast<squares::Index>(target - pd.single_push_direction());
    maybe_promoting(moves, state, source, target, move_types::normal);
  }

  if (targets & pd.double_push_targets(state.board[state.us][pieces::pawn], state.flat_occupancy)) {
    squares::Index source = static_cast<squares::Index>(target - pd.double_push_direction());
    maybe_promoting(moves, state, source, target, move_types::double_push);
  }
}

void moves::pawn_moves_capturing(std::vector<Move>& moves, State const& state, squares::Index target) {
  Bitboard targets = squares::bitboard(target);

  if (state.en_passant_square) {
    Bitboard double_pushed_pawn = state.us == colors::white
      ? state.en_passant_square >> directions::vertical
      : state.en_passant_square << directions::vertical;
    if (targets & double_pushed_pawn) {
      targets |= state.en_passant_square;
    }
  }

  targets::PawnDingbat const& pd = targets::pawn_dingbats[state.us];
  for (const targets::PawnAttackType& pa: targets::pawn_attack_types) {
    signed direction = pd.leftshift - pd.rightshift + pa.leftshift - pa.rightshift;
    squares::for_each(pawn_attacks(state.board[state.us][pieces::pawn], pd, pa) & targets,
                      [&](squares::Index target) {
                        squares::Index source = static_cast<squares::Index>(target - direction);
                        maybe_promoting(moves, state, source, target, move_types::capture);
                      });
  }
}

typedef void(*MoveGenerator)(std::vector<Move>&, State const&, Bitboard);
const static MoveGenerator piecewise_move_generators[] = {
  moves::pawn_moves,
  moves::knight_moves,
  moves::bishop_moves,
  moves::rook_moves,
  moves::queen_moves,
  moves::king_moves,
};

// generates pseudolegal moves.  moves that leave the king in check may be
// generated.  the opponent's only moves will be king captures.
void moves::moves(std::vector<Move>& moves, State const& state) {
  if (state.game_definitely_over())
    return;

  if (state.their_king_attacked()) {
    moves::capturing(moves, state, squares::index(state.board[state.them][pieces::king]));
    return;
  }

  moves.reserve(50);

  if (state.in_check()) {
    check_evading_moves(moves, state);
  } else {
    for (Piece piece: pieces::values)
      (*piecewise_move_generators[piece])(moves, state, state.board[state.us][piece]);

    castle_moves(moves, state);
  }
}

std::vector<Move> moves::moves(State const& state) {
  std::vector<Move> result;
  moves(result, state);
  return result;
}

// NOTE: pseudolegal
void moves::check_evading_moves(std::vector<Move>& moves, State const& state) {
  Bitboard bbking = state.board[state.us][pieces::king]; // hah!
  squares::Index king = squares::index(bbking);

  king_moves(moves, state, bbking);

  Bitboard attackers = targets::attackers(bbking, state.flat_occupancy, state.them, state.board[state.them]);
  if (bitboard::cardinality(attackers) == 1) {
    squares::Index attacker = squares::index(attackers);
    moves::capturing(moves, state, attacker, true);
    squares::for_each(squares::in_between(attacker, king), [&](squares::Index target) {
        moves::occupying(moves, state, target, true);
      });
  }
}

void moves::legal_moves(std::vector<Move>& moves, State& state) {
  moves::moves(moves, state);
  erase_illegal_moves(moves, state);
}

void moves::erase_illegal_moves(std::vector<Move>& moves, State& state) {
  for (auto it = moves.begin(); it != moves.end(); ) {
    Undo undo = state.make_move(*it);
    if (state.their_king_attacked()) {
      it = moves.erase(it);
    } else {
      it++;
    }
    state.unmake_move(undo);
  }
}

boost::optional<Move> moves::maybe_fast_random_move(State const& state, boost::mt19937& generator) {
  squares::Index source = squares::random_index(state.occupancy[state.us], generator);
  Piece piece = state.piece_at(source, state.us);
  std::vector<Move> moves;
  (*piecewise_move_generators[piece])(moves, state, state.board[state.us][piece]);
  if (moves.empty())
    return boost::none;
  return random_element(moves, generator);
}

boost::optional<Move> moves::random_move(State const& state, boost::mt19937& generator) {
  if (state.game_definitely_over())
    return boost::none;

  // maybe_fast_random_move doesn't always find king captures if they are
  // available.
  if (!state.in_check() && !state.their_king_attacked()) {
    // try the fast method a couple of times; if it fails repeatedly then there
    // may be many pieces with no moves available to them.  then just generate
    // all moves and select one of those.
    for (int i = 0; i < 3; i++) {
      boost::optional<Move> move = maybe_fast_random_move(state, generator);
      if (move)
        return move;
    }
  }

  std::vector<Move> moves;
  moves::moves(moves, state);
  if (moves.empty())
    return boost::none;
  return random_element(moves, generator);
}

boost::optional<Move> moves::maybe_make_fast_random_legal_move(State& state, boost::mt19937& generator) {
  boost::optional<Move> move = maybe_fast_random_move(state, generator);
  if (!move)
    return boost::none;
  Undo undo = state.make_move(*move);
  if (state.their_king_attacked()) {
    state.unmake_move(undo);
    return boost::none;
  } else {
    return move;
  }
}

boost::optional<Move> moves::make_random_legal_move(State& state, boost::mt19937& generator) {
  if (state.game_definitely_over())
    return boost::none;

  if (!state.in_check()) {
    for (int i = 0; i < 3; i++) {
      boost::optional<Move> move = maybe_make_fast_random_legal_move(state, generator);
      if (move)
        return move;
    }
  }

  // if no move was found, generate all legal moves and choose from there
  std::vector<Move> moves;
  legal_moves(moves, state);
  if (moves.empty())
    return boost::none;

  Move move = random_element(moves, generator);
  state.make_move(move);
  return move;
}
