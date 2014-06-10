#include <stdexcept>
#include <cassert>

#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include "move_generation.hpp"
#include "direction.hpp"
#include "targets.hpp"
#include "state.hpp"

// Generate moves from the set of targets.  source_fn computes the source from the target index
template <typename F>
void moves_from_targets(std::vector<Move>& moves, Bitboard targets, F source_fn, MoveType type) {
  squares::for_each(targets, [&](squares::Index target) {
    moves.emplace_back(source_fn(target), target, type);
  });
}

// Generate normal and capturing moves from the set of attacks.
template <typename F>
void moves_from_attacks(std::vector<Move>& moves, Bitboard attacks,
                        Bitboard us, Bitboard them,
                        F source_fn) {
  moves_from_targets(moves, attacks & ~us & ~them, source_fn, move_types::normal);
  assert((us & them) == 0);
  moves_from_targets(moves, attacks & them,        source_fn, move_types::capture);
}


// if any of the piecewise move generators detects that the opponent's king
// can be captured, it will return with the moves vector containing only the
// king capture.
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
// generated.  the opponent's only move will be to capture the king.
void moves::moves(std::vector<Move>& moves, State const& state) {
  if (state.game_definitely_over())
    return;

  moves.reserve(50);

  for (Piece piece: pieces::values) {
    (*piecewise_move_generators[piece])(moves, state, state.board[state.us][piece]);
    if (moves.size() == 1 && moves[0].is_king_capture())
      return;
  }

  castle_moves(moves, state);
}

std::vector<Move> moves::moves(State const& state) {
  std::vector<Move> result;
  moves(result, state);
  return result;
}

template <typename F>
void slider_moves(std::vector<Move>& moves, State const& state, Bitboard sources, F attack_getter) {
  squares::for_each(sources, [&](squares::Index source) {
    Bitboard targets = attack_getter(source, state.flat_occupancy);

    if (targets & state.board[state.them][pieces::king]) {
      squares::Index target = squares::index(targets & state.board[state.them][pieces::king]);
      moves = {Move(source, target, move_types::king_capture)};
      return;
    }

    moves_from_attacks(moves, targets,
                       state.occupancy[state.us],
                       state.occupancy[state.them],
                       [&](squares::Index target) { return source; });
  });
}

void moves::king_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  Bitboard targets = targets::king_attacks(sources) & ~state.their_attacks;
  squares::Index source = squares::index(sources);
  moves_from_attacks(moves, targets, 
                     state.occupancy[state.us],
                     state.occupancy[state.them],
                     [&](squares::Index target) { return source; });
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
  for (const targets::KnightAttackType& ka: targets::get_knight_attack_types()) {
    Bitboard targets = ka.attacks(sources);

    auto source_fn = [&](squares::Index target) {
      return static_cast<squares::Index>(target - ka.direction());
    };

    if (targets & state.board[state.them][pieces::king]) {
      squares::Index target = squares::index(targets & state.board[state.them][pieces::king]);
      moves = {Move(source_fn(target), target, move_types::king_capture)};
      return;
    }

    moves_from_attacks(moves, targets,
                       state.occupancy[state.us],
                       state.occupancy[state.them],
                       source_fn);
  }
}

void moves::pawn_moves(std::vector<Move>& moves, State const& state, Bitboard sources) {
  // TODO: clean up this mess (though pawn behavior is inherently complicated -_-)

  const targets::PawnDingbat &pd = targets::get_pawn_dingbats()[state.us];

  // single push
  auto single_source_fn = [&](squares::Index target) {
    return static_cast<squares::Index>(target - pd.single_push_direction());
  };
  Bitboard single_push_targets = pd.single_push_targets(sources, state.flat_occupancy);
  moves_from_targets(moves, single_push_targets & ~pd.promotion_rank, single_source_fn, move_types::normal);
  Bitboard promotion_targets = single_push_targets & pd.promotion_rank;
  if (promotion_targets) {
    for (MoveType promotion: {move_types::promotion_knight, move_types::promotion_bishop,
                              move_types::promotion_rook,   move_types::promotion_queen}) {
      moves_from_targets(moves, promotion_targets, single_source_fn, promotion);
    }
  }

  // double push
  Bitboard double_push_targets = pd.double_push_targets(sources, state.flat_occupancy);
  moves_from_targets(moves, double_push_targets,
                            [&](squares::Index target) {
                              return static_cast<squares::Index>(target - pd.double_push_direction());
                            },
                            move_types::double_push);

  // captures
  for (const targets::PawnAttackType& pa: targets::get_pawn_attack_types()) {
    signed direction = pd.leftshift - pd.rightshift + pa.leftshift - pa.rightshift;

    auto capture_source_fn = [&](squares::Index target) {
      return static_cast<squares::Index>(target - direction);
    };

    Bitboard capture_targets = pawn_attacks(sources, pd, pa) & (state.occupancy[state.them] | state.en_passant_square);

    if (capture_targets & state.board[state.them][pieces::king]) {
      squares::Index target = squares::index(capture_targets & state.board[state.them][pieces::king]);
      moves = {Move(capture_source_fn(target), target, move_types::king_capture)};
      return;
    }

    moves_from_targets(moves, capture_targets & ~pd.promotion_rank, capture_source_fn, move_types::capture);
    promotion_targets = capture_targets & pd.promotion_rank;
    if (promotion_targets != 0) {
      for (MoveType promotion: {move_types::capturing_promotion_knight, move_types::capturing_promotion_bishop,
                                move_types::capturing_promotion_rook,   move_types::capturing_promotion_queen}) {
        moves_from_targets(moves, promotion_targets, capture_source_fn, promotion);
      }
    }
  }
}

void moves::castle_moves(std::vector<Move>& moves, State const& state) {
  for (Castle castle: castles::values) {
    // TODO: extract method state.can_castle(castle) which calls out to targets::is_attacked etc
    if (state.castling_rights[state.us][castle]
        && !(castles::safe_squares(state.us, castle) & state.their_attacks)
        && !(castles::free_squares(state.us, castle) & state.flat_occupancy))
      moves.push_back(Move::castle(state.us, castle));
  }
}

void moves::legal_moves(std::vector<Move>& moves, State& state) {
  moves::moves(moves, state);
  erase_illegal_moves(moves, state);
}

void moves::erase_illegal_moves(std::vector<Move>& moves, State& state) {
  for (auto it = moves.begin(); it != moves.end(); ) {
    Undo undo = state.make_move(*it);
    if (state.their_king_in_check()) {
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
  if (!state.their_king_in_check()) {
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
  if (state.their_king_in_check()) {
    state.unmake_move(undo);
    return boost::none;
  } else {
    return move;
  }
}

boost::optional<Move> moves::make_random_legal_move(State& state, boost::mt19937& generator) {
  if (state.game_definitely_over())
    return boost::none;

  // try a pseudolegal move
  for (int i = 0; i < 5; i++) {
    boost::optional<Move> move = maybe_make_fast_random_legal_move(state, generator);
    if (move)
      return move;
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
