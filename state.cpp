#include <string>
#include <cctype>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "state.hpp"
#include "direction.hpp"

State::State() {
  set_initial_configuration();
}

State::State(std::string fen) {
  load_fen(fen);
}

void State::set_initial_configuration() {
  using namespace colors;
  using namespace pieces;
  using namespace squares::bitboards;
  using namespace ranks::bitboards;

  board[white][pawn] = _2;
  board[white][knight] = b1 | g1;
  board[white][bishop] = c1 | f1;
  board[white][rook] = a1 | h1;
  board[white][queen] = d1;
  board[white][king] = e1;

  board[black][pawn] = _7;
  board[black][knight] = b8 | g8;
  board[black][bishop] = c8 | f8;
  board[black][rook] = a8 | h8;
  board[black][queen] = d8;
  board[black][king] = e8;

  for (Color color: colors::values)
    for (Castle castle: castles::values)
      castling_rights[color][castle] = true;

  en_passant_square = 0;

  us = white; them = black;

  halfmove_clock = 0;

  compute_occupancy();
  compute_their_attacks();
  compute_hash();
}

std::string State::dump_fen() {
  std::stringstream ss;
  for (ranks::Index rank: boost::adaptors::reverse(ranks::indices)) {
    size_t empty_count = 0;
    for (files::Index file: files::indices) {
      squares::Index square = squares::index(ranks::bitboard(rank) & files::bitboard(file));
      boost::optional<ColoredPiece> colored_piece = colored_piece_at(square);
      if (colored_piece) {
        if (empty_count > 0) {
          ss << empty_count;
          empty_count = 0;
        }
        ss << colored_piece->symbol();
      } else {
        empty_count++;
      }
    }
    if (empty_count > 0)
      ss << empty_count;
    if (rank != ranks::_1)
      ss << "/";
  }
  ss << " ";
  ss << colors::symbol(us);
  ss << " ";
  for (Color color: colors::values) {
    for (Castle castle: castles::values) {
      if (castling_rights[color][castle]) {
        ss << castles::symbol(color, castle);
      }
    }
  }
  ss << " ";
  if (en_passant_square)
    ss << squares::keywords[squares::index(en_passant_square)];
  else
    ss << "-";
  ss << " ";
  ss << halfmove_clock;
  ss << " ";
  ss << 0;
  return ss.str();
}

void State::load_fen(std::string fen) {
  boost::regex fen_regex("((\\w+/){7}\\w+)\\s+([bw])\\s+((K)?(Q)?(k)?(q)?|-)\\s+(([a-h][1-8])|-)\\s+(\\d+)\\s+.*");
  boost::smatch m;
  if (!boost::regex_match(fen, m, fen_regex))
    throw std::runtime_error(str(boost::format("can't parse FEN: %1%") % fen));

  std::string board(m[1].first, m[1].second);

  boost::regex rank_separator("/");
  std::vector<std::string> fen_ranks;
  boost::algorithm::split_regex(fen_ranks, std::string(m[1].first, m[1].second), rank_separator);
  std::reverse(fen_ranks.begin(), fen_ranks.end());

  using namespace pieces;
  empty_board();
  squares::Index square = squares::a1;
  for (ranks::Index rank: ranks::indices) {
    for (char c: fen_ranks[rank]) {
      if (isdigit(c)) {
        short n = boost::lexical_cast<short>(c);
        assert(1 <= n && n <= 8);
        square = static_cast<squares::Index>(square + n);
      } else {
        Color owner = islower(c) ? colors::black : colors::white;
        c = tolower(c);
        Bitboard board = squares::bitboard(square);
        switch (c) {
        case 'p': this->board[owner][pawn]   |= board; break;
        case 'n': this->board[owner][knight] |= board; break;
        case 'b': this->board[owner][bishop] |= board; break;
        case 'r': this->board[owner][rook]   |= board; break;
        case 'q': this->board[owner][queen]  |= board; break;
        case 'k': this->board[owner][king]   |= board; break;
        default: throw std::runtime_error(str(boost::format("invalid FEN piece symbol %1% in FEN \"%2%\"") % c % fen));
        }
        square = static_cast<squares::Index>(square + 1);
      }
    }

    if (square != (rank + 1) * 8)
      throw std::runtime_error(str(boost::format("rank %1% has width unequal to 8 in FEN \"%2%\"") % ranks::keywords[rank] % fen));
  }

  Color color_to_move = std::string(m[3].first, m[3].second) == "w" ? colors::white : colors::black;

  castling_rights[colors::white][castles::kingside]  = m[5].matched;
  castling_rights[colors::white][castles::queenside] = m[6].matched;
  castling_rights[colors::black][castles::kingside]  = m[7].matched;
  castling_rights[colors::black][castles::queenside] = m[8].matched;

  if (m[10].matched) {
    en_passant_square = squares::bitboard(squares::by_keyword(std::string(m[10].first, m[10].second)));
  } else {
    en_passant_square = 0;
  }

  us = color_to_move;
  them = us == colors::white ? colors::black : colors::white;

  halfmove_clock = std::stoi(std::string(m[11].first, m[11].second));

  compute_occupancy();
  compute_their_attacks();
  compute_hash();
}

State::State(const State &that) :
  us(that.us),
  them(that.them),
  board(that.board),
  castling_rights(that.castling_rights),
  en_passant_square(that.en_passant_square),
  occupancy(that.occupancy),
  their_attacks(that.their_attacks),
  hash(that.hash),
  halfmove_clock(that.halfmove_clock)
{
}

State::~State() {}

void State::empty_board() {
  for (Color c: colors::values)
    for (Piece p: pieces::values)
      board[c][p] = 0;
}

bool State::operator==(const State &that) const {
  if (this == &that)
    return true;
  
  // NOTE: the halfmove clock is not compared. it is really only used to cut
  // short simulations.
  return this->hash == that.hash &&
         this->us == that.us && this->them == that.them &&
         this->en_passant_square == that.en_passant_square &&
         this->castling_rights == that.castling_rights &&
         this->board == that.board;
}

std::ostream& operator<<(std::ostream& o, const State& s) {
  Board board(s.board);
  // squares run from white to black, output runs from top to bottom.  flip
  // the board to get white on the bottom.
  board::flip_vertically(board);

  for (squares::Index square: squares::indices) {
    o << ' ';

    bool piece_found = false;
    for (Color c: colors::values) {
      for (Piece p: pieces::values) {
        if (board[c][p] & squares::bitboard(square)) {
          o << ColoredPiece(c, p).symbol();

          // take this opportunity to ensure no two pieces are on the same square
          assert(!piece_found);
          piece_found = true;
        }
      }
    }

    if (!piece_found)
      o << '.';

    if ((square + 1) % 8 == 0)
      o << std::endl;
  }

  o << colors::name(s.us) << " to move.";

  o << " castling rights: ";
  for (Color color: colors::values) {
    for (Castle castle: castles::values) {
      if (s.castling_rights[color][castle])
        o << castles::symbol(color, castle);
    }
  }

  if (s.en_passant_square != 0)
    o << " en-passant square: " << squares::keywords[squares::index(s.en_passant_square)];

  o << std::endl;
  return o;
}

std::vector<Move> State::moves() const {
  // TODO: maybe reserve()
  std::vector<Move> moves;
  moves::moves(moves, us, them,
               board, occupancy,
               their_attacks, en_passant_square,
               castling_rights);
  return moves;
}

std::vector<Move> State::legal_moves() {
  std::vector<Move> moves = this->moves();
  erase_illegal_moves(moves);
  return moves;
}

void State::erase_illegal_moves(std::vector<Move>& moves) {
  for (auto it = moves.begin(); it != moves.end(); ) {
    Undo undo = make_move(*it);
    if (their_king_in_check()) {
      it = moves.erase(it);
    } else {
      it++;
    }
    unmake_move(undo);
  }
}

boost::optional<Move> State::random_move(boost::mt19937& generator) const {
  // TODO: do better
  std::vector<Move> moves = this->moves();
  if (moves.empty())
    return boost::none;
  return random_element(moves, generator);
}

boost::optional<Move> State::make_random_legal_move(boost::mt19937& generator) {
  // try a pseudolegal move
  std::vector<Move> moves = this->moves();
  if (moves.empty())
    return boost::none;
  Move move = random_element(moves, generator);
  Undo undo = make_move(move);

  // if it was legal, fine
  if (!their_king_in_check())
    return move;

  // if it was not legal (happens relatively rarely), narrow down to legal moves
  unmake_move(undo);
  erase_illegal_moves(moves);

  if (moves.empty())
    return boost::none;
  return random_element(moves, generator);
}

bool State::their_king_in_check() const {
  // TODO: optimize
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  return moves::attacks(us, flat_occupancy, board[us]) & board[them][pieces::king];
}

boost::optional<ColoredPiece> State::colored_piece_at(squares::Index square) const {
  Bitboard bitboard = squares::bitboard(square);
  for (Color color: colors::values)
    for (Piece piece: pieces::values)
      if (board[color][piece] & bitboard)
        return ColoredPiece(color, piece);
  return boost::none;
}

Piece State::piece_at(squares::Index square, Color color) const {
  Bitboard bitboard = squares::bitboard(square);
  for (Piece piece: pieces::values)
    if (board[color][piece] & bitboard)
      return piece;
  std::cerr << "State::piece_at: no " << colors::name(color) << " piece at " << squares::keywords.at(square) << " in state: " << std::endl;
  std::cerr << *this << std::endl;
  print_backtrace();
  throw std::runtime_error("no such piece");
}

void State::update_castling_rights(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target) {
  undo.prior_castling_rights = castling_rights;

  switch (piece) {
  case pieces::king:
    for (Castle castle: castles::values) {
      if (castling_rights[us][castle]) {
        castling_rights[us][castle] = false;
        hash ^= hashes::can_castle(us, castle);
      }
    }
    break;
  case pieces::rook:
    {
      boost::optional<Castle> castle = castles::involving(move.source(), us);
      if (castle && castling_rights[us][*castle]) {
        castling_rights[us][*castle] = false;
        hash ^= hashes::can_castle(us, *castle);
      }
    }
    break;
  case pieces::pawn:
  case pieces::knight:
  case pieces::bishop:
  case pieces::queen:
    break;
  default:
    throw std::runtime_error(str(boost::format("unhandled Piece case: %|1$#x|") % piece));
  }
}

void State::update_en_passant_square(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target) {
  undo.prior_en_passant_square = en_passant_square;

  if (en_passant_square)
    hash ^= hashes::en_passant(en_passant_square);

  switch (move.type()) {
  case move_types::double_push:
    assert(piece == pieces::pawn);
    en_passant_square = us == colors::white ? target >> directions::vertical
                                            : target << directions::vertical;
    hash ^= hashes::en_passant(en_passant_square);
    break;
  case move_types::capture:
  case move_types::king_capture:
  case move_types::castle_kingside:
  case move_types::castle_queenside:
  case move_types::promotion_knight:
  case move_types::promotion_bishop:
  case move_types::promotion_rook:
  case move_types::promotion_queen:
  case move_types::capturing_promotion_knight:
  case move_types::capturing_promotion_bishop:
  case move_types::capturing_promotion_rook:
  case move_types::capturing_promotion_queen:
  case move_types::normal:
    en_passant_square = 0;
    break;
  default:
    throw std::runtime_error(str(boost::format("unhandled MoveType case: %|1$#x|") % move.type()));
  }
}

void State::make_move_on_their_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target) {
  using hashes::toggle;

  Halfboard& their_halfboard = board[them];

  switch (move.type()) {
  case move_types::capturing_promotion_knight:
  case move_types::capturing_promotion_bishop:
  case move_types::capturing_promotion_rook:
  case move_types::capturing_promotion_queen:
  case move_types::capture:
  case move_types::king_capture:
    if (target == en_passant_square) {
      assert(piece == pieces::pawn);
      // the captured pawn is in front of the en_passant_square
      Bitboard capture_target = us == colors::white ? target >> directions::vertical
                                                    : target << directions::vertical;
      assert(their_halfboard[pieces::pawn] & capture_target);

      their_halfboard[pieces::pawn] &= ~capture_target;
      toggle(hash, them, pieces::pawn, squares::index(capture_target));

      undo.record_capture(pieces::pawn, capture_target);
    } else {
      // if we need speed, break out of the loop as soon as we find the piece
      // we're looking for.  otherwise keep going to test the assumption that
      // exactly one piece was there.
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
      Halfboard their_prior_halfboard = Halfboard(their_halfboard);
      bool found = false;
#endif
      for (Piece capturee: pieces::values) {
        if (their_halfboard[capturee] & target) {
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
          assert(!found /* no more than on piece on this square? */);
          found = true;
#endif
          their_halfboard[capturee] &= ~target;
          toggle(hash, them, capturee, squares::index(target));

          undo.record_capture(capturee, target);
#ifndef MC_EXPENSIVE_RUNTIME_TESTS
          break;
#endif
        }
      }
#ifdef MC_EXPENSIVE_RUNTIME_TESTS
      assert(found == true /* at least one piece on this square? */);
#endif
    }
    break;
  case move_types::double_push:
  case move_types::castle_kingside:
  case move_types::castle_queenside:
  case move_types::promotion_knight:
  case move_types::promotion_bishop:
  case move_types::promotion_rook:
  case move_types::promotion_queen:
  case move_types::normal:
    undo.record_no_capture();
    break;
  default:
    throw std::runtime_error(str(boost::format("unhandled MoveType case: %|1$#x|") % move.type()));
  }
}

void State::make_move_on_our_halfboard(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target) {
  using namespace pieces;
  using hashes::toggle;

  Halfboard& our_halfboard = board[us];

  assert(our_halfboard[piece] & source);

  our_halfboard[piece] &= ~source; toggle(hash, us, piece, move.source());
  our_halfboard[piece] |=  target; toggle(hash, us, piece, move.target());

  switch (move.type()) {
  case move_types::castle_kingside:
  case move_types::castle_queenside:
    {
      squares::Index rook0 = castles::rook_source(move.target());
      squares::Index rook1 = castles::rook_target(move.target());
      our_halfboard[rook] &= ~squares::bitboard(rook0); toggle(hash, us, rook, rook0);
      our_halfboard[rook] |=  squares::bitboard(rook1); toggle(hash, us, rook, rook1);
    }
    break;
  case move_types::capturing_promotion_knight:
  case move_types::promotion_knight:
    assert(piece == pawn);
    our_halfboard[piece]  &= ~target; toggle(hash, us, pawn,   move.target());
    our_halfboard[knight] |=  target; toggle(hash, us, knight, move.target());
    break;
  case move_types::capturing_promotion_bishop:
  case move_types::promotion_bishop:
    assert(piece == pawn);
    our_halfboard[piece]  &= ~target; toggle(hash, us, pawn,   move.target());
    our_halfboard[bishop] |=  target; toggle(hash, us, bishop, move.target());
    break;
  case move_types::capturing_promotion_rook:
  case move_types::promotion_rook:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target; toggle(hash, us, pawn,  move.target());
    our_halfboard[rook]  |=  target; toggle(hash, us, rook,  move.target());
    break;
  case move_types::capturing_promotion_queen:
  case move_types::promotion_queen:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target; toggle(hash, us, pawn,  move.target());
    our_halfboard[queen] |=  target; toggle(hash, us, queen, move.target());
    break;
  case move_types::capture:
  case move_types::king_capture:
  case move_types::double_push:
  case move_types::normal:
    break;
  default:
    throw std::runtime_error(str(boost::format("unhandled MoveType case: %|1$#x|") % move.type()));
  }
}

void State::make_move_on_occupancy(const Move& move, Undo& undo, const Piece piece, const Bitboard source, const Bitboard target) {
  occupancy[us] &= ~source;
  occupancy[us] |=  target;

  switch (move.type()) {
  case move_types::capturing_promotion_knight:
  case move_types::capturing_promotion_bishop:
  case move_types::capturing_promotion_rook:
  case move_types::capturing_promotion_queen:
  case move_types::capture:
  case move_types::king_capture:
    {
      Bitboard capture_target = target;
      if (target == en_passant_square) {
        capture_target = (us == colors::white
                          ? target >> directions::vertical
                          : target << directions::vertical);
      }
      assert(occupancy[them] & capture_target);
      occupancy[them] &= ~capture_target;
    }
    break;
  case move_types::castle_kingside:
  case move_types::castle_queenside:
    occupancy[us] &= ~squares::bitboard(castles::rook_source(move.target()));
    occupancy[us] |=  squares::bitboard(castles::rook_target(move.target()));
    break;
  case move_types::double_push:
  case move_types::promotion_knight:
  case move_types::promotion_bishop:
  case move_types::promotion_rook:
  case move_types::promotion_queen:
  case move_types::normal:
    break;
  default:
    throw std::runtime_error(str(boost::format("unhandled MoveType case: %|1$#x|") % move.type()));
  }
}

Undo State::make_move(const Move& move) {
  Undo undo;
  undo.move = move;

  Bitboard source = squares::bitboard(move.source()),
           target = squares::bitboard(move.target());
  Piece piece = piece_at(move.source(), us);

#ifdef MC_EXPENSIVE_RUNTIME_TESTS
  State prior_state(*this);
#endif

  make_move_on_our_halfboard   (move, undo, piece, source, target);
  make_move_on_their_halfboard (move, undo, piece, source, target);
  make_move_on_occupancy       (move, undo, piece, source, target);
  update_en_passant_square     (move, undo, piece, source, target);
  update_castling_rights       (move, undo, piece, source, target);

  std::swap(us, them);
  hash ^= hashes::black_to_move();

  undo.prior_halfmove_clock = halfmove_clock;
  if (piece == pieces::pawn || move.is_capture()) {
    halfmove_clock = 0;
  } else {
    halfmove_clock++;
  }

  compute_their_attacks();
  return undo;
}

void State::unmake_move(const Undo& undo) {
  std::swap(us, them);

  Piece piece = piece_at(undo.move.target(), us);

  // move back our piece
  board[us][piece] &= ~squares::bitboard(undo.move.target());
  if (undo.move.is_promotion()) {
    // demote
    board[us][pieces::pawn] |= squares::bitboard(undo.move.source());
  } else {
    board[us][piece]        |= squares::bitboard(undo.move.source());
  }

  // in case of castle, move the rook back as well
  if (undo.move.is_castle()) {
    board[us][pieces::rook] = board[us][pieces::rook]
      & ~squares::bitboard(castles::rook_target(undo.move.target()))
      |  squares::bitboard(castles::rook_source(undo.move.target()));
  }

  // replace any captured piece
  board[them][undo.captured_piece] |= undo.capture_square;

  halfmove_clock = undo.prior_halfmove_clock;
  en_passant_square = undo.prior_en_passant_square;
  castling_rights = undo.prior_castling_rights;

  compute_hash();
  compute_occupancy();
  compute_their_attacks();
}

void State::compute_occupancy()     { compute_occupancy(occupancy); }
void State::compute_their_attacks() { compute_their_attacks(their_attacks); }
void State::compute_hash()          { compute_hash(hash); }

void State::compute_occupancy(Occupancy& occupancy) {
  board::flatten(board, occupancy);
}

void State::compute_their_attacks(Bitboard& their_attacks) {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  their_attacks = moves::attacks(them, flat_occupancy, board[them]);
}

void State::compute_hash(Hash &hash) {
  hash = 0;

  for (Color c: colors::values) {
    for (Piece p: pieces::values) {
      squares::do_bits(board[c][p], [this, &c, &p, &hash](squares::Index si) {
          hash ^= hashes::colored_piece_at_square(c, p, si);
        });
    }

    for (Castle castle: castles::values)
      if (castling_rights[c][castle])
        hash ^= hashes::can_castle(c, castle);
  }

  if (us == colors::black)
    hash ^= hashes::black_to_move();

  if (en_passant_square)
    hash ^= hashes::en_passant(en_passant_square);
}

bool State::drawn_by_50() const {
  return halfmove_clock >= 50;
}

// NOTE: assumes game is over and no moves have been made since the game was over
boost::optional<Color> State::winner() const {
  if (drawn_by_50())
    return boost::none;
  return them;
}
