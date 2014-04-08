#include <string>
#include <cctype>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/format.hpp>

#include "state.hpp"
#include "direction.hpp"

State::State() :
  us(colors::white), them(colors::black),
  en_passant_square(0),
  occupancy({0x000000000000ffff, 0xffff000000000000}),
  their_attacks(0xe7ffff0000000000)
{
  using namespace colors;
  using namespace pieces;
  using namespace squares;

  board[white][pawn] = ranks::_2;
  board[white][knight] = b1 | g1;
  board[white][bishop] = c1 | f1;
  board[white][rook] = a1 | h1;
  board[white][queen] = d1;
  board[white][king] = e1;

  board[black][pawn] = ranks::_7;
  board[black][knight] = b8 | g8;
  board[black][bishop] = c8 | f8;
  board[black][rook] = a8 | h8;
  board[black][queen] = d8;
  board[black][king] = e8;

  can_castle_kingside.fill(true);
  can_castle_queenside.fill(true);
}

State::State(std::string fen)
{
  boost::regex fen_regex("((\\w+/){7}\\w+)\\s+([bw])\\s+((K)?(Q)?(k)?(q)?|-)\\s+(([a-h][1-8])|-)\\s+.*");
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
  squares::Index square_index = 0;
  for (Rank rank: ranks::partition) {
    assert(square_index == rank.index * 8);
    for (char c: fen_ranks[rank.index]) {
      if (isdigit(c)) {
        short n = boost::lexical_cast<short>(c);
        assert(1 <= n && n <= 8);
        square_index += n;
      } else {
        Color owner = islower(c) ? colors::black : colors::white;
        c = tolower(c);
        Bitboard square = squares::partition[square_index];
        switch (c) {
        case 'p': this->board[owner][pawn]   |= square; break;
        case 'n': this->board[owner][knight] |= square; break;
        case 'b': this->board[owner][bishop] |= square; break;
        case 'r': this->board[owner][rook]   |= square; break;
        case 'q': this->board[owner][queen]  |= square; break;
        case 'k': this->board[owner][king]   |= square; break;
        default: throw std::runtime_error(str(boost::format("invalid FEN piece symbol %1% in FEN \"%2%\"") % c % fen));
        }
        square_index++;
      }
    }
  }
  assert(square_index == 64);

  Color color_to_move = std::string(m[3].first, m[3].second) == "w" ? colors::white : colors::black;

  can_castle_kingside  = {m[5].matched, m[7].matched};
  can_castle_queenside = {m[6].matched, m[8].matched};

  if (m[10].matched)
    en_passant_square = squares::partition[std::string(m[10].first, m[10].second)];

  us = colors::white;
  them = colors::black;

  compute_occupancy();

  if (color_to_move == colors::white)
    flip_perspective();
  compute_attacks();
  flip_perspective();
}

State::State(State &that) :
  us(that.us),
  them(that.them),
  board(that.board),
  can_castle_kingside(that.can_castle_kingside),
  can_castle_queenside(that.can_castle_queenside),
  en_passant_square(that.en_passant_square),
  occupancy(that.occupancy),
  their_attacks(that.their_attacks)
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
  
  return this->us == that.us && this->them == that.them &&
         this->en_passant_square == that.en_passant_square &&
         this->can_castle_kingside == that.can_castle_kingside &&
         this->can_castle_queenside == that.can_castle_queenside &&
         this->board == that.board;
}

std::ostream& operator<<(std::ostream& o, const State& s) {
  // FIXME: deal with monochromicity
  array2d<char, colors::cardinality, pieces::cardinality> symbols = {
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k',
  };

  for (Square square: squares::partition) {
    o << ' ';

    squares::Index i = square.index;
    // bit index with rank flipped
    // TODO: this can be done more easily, and take us/them into account
    size_t j = ((7 - (i >> 3)) << 3) + (i & 7);

    Bitboard bitboard = Bitboard(1)<<j;
    bool piece_found = false;
    for (Color c: colors::values) {
      for (Piece p: pieces::values) {
        if (s.board[c][p] & bitboard) {
          o << symbols[c][p];

          // take this opportunity to ensure no two pieces are on the same square
          assert(!piece_found);
          piece_found = true;
        }
      }
    }

    if (!piece_found)
      o << '.';

    if ((i + 1) % 8 == 0)
      o << std::endl;
  }

  o << "from " << colors::name(s.us) << "'s perspective. ";

  o << " castling rights: ";
  if (s.can_castle_kingside [colors::white]) o << "k";
  if (s.can_castle_queenside[colors::white]) o << "q";
  if (s.can_castle_kingside [colors::black]) o << "K";
  if (s.can_castle_queenside[colors::black]) o << "Q";

  if (s.en_passant_square != 0)
    o << " en-passant square: " << squares::from_bitboard(s.en_passant_square).name;

  o << std::endl;
  return o;
}

bool State::leaves_king_in_check(const Move& m) const {
  const Square source = squares::partition[m.from()],
               target = squares::partition[m.to()];

  // king move?
  if (source & board[us][pieces::king])
    return !(their_attacks & target);

  // only occupancy and their halfboard make a difference
  Occupancy occupancy(this->occupancy);
  make_move_on_occupancy(m, source, target, occupancy);
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);

  Halfboard their_halfboard(board[them]);
  make_move_on_their_halfboard(m, moving_piece(m, board[us]), source, target, their_halfboard);

  return moves::all_attacks(flat_occupancy, their_halfboard) & board[us][pieces::king];
}

std::vector<Move> State::moves() const {
  // TODO: maybe reserve()
  std::vector<Move> moves;
  moves::all_moves(moves, board, occupancy, en_passant_square,
                   can_castle_kingside [us],
                   can_castle_queenside[us]);

  // filter out moves that would leave the king in check
  for (auto it = moves.begin(); it != moves.end(); ) {
    if (leaves_king_in_check(*it)) {
      it = moves.erase(it);
    } else {
      it++;
    }
  }

  return moves;
}

Move State::parse_algebraic(std::string algebraic) const {
  // FIXME: deal with monochromicity; flip ranks based on color_to_move
  boost::regex algebraic_move_regex("([NBRQK]?)([a-h])?([1-8])?(x)?([a-h][1-8])+?");
  boost::smatch m;
  if (!boost::regex_match(algebraic, m, algebraic_move_regex))
    throw std::runtime_error(str(boost::format("can't parse algebraic move: %1%") % algebraic));

  Piece piece = pieces::type_from_name(std::string(m[1].first, m[1].second));

  boost::optional<File> source_file;
  boost::optional<Rank> source_rank;
  if (m[2].matched) source_file = files::partition[std::string(m[2].first, m[2].second)];
  if (m[3].matched) source_rank = ranks::partition[std::string(m[3].first, m[3].second)];

  bool is_capture = m[4].matched;
  squares::Index target = squares::partition[std::string(m[5].first, m[5].second)].index;

  std::vector<Move> candidates = match_algebraic(piece, source_file, source_rank, is_capture, target);
  if (candidates.empty())
    throw std::runtime_error(str(boost::format("no match for algebraic move: %1%") % algebraic));
  if (candidates.size() > 1)
    throw std::runtime_error(str(boost::format("ambiguous algebraic move: %1%, candidates: %2%") % algebraic % candidates));
  return candidates[0];
}

std::vector<Move> State::match_algebraic(const Piece piece,
                                         boost::optional<File> source_file,
                                         boost::optional<Rank> source_rank,
                                         const bool is_capture,
                                         const squares::Index target) const {
  std::vector<Move> candidates;
  moves::piece_moves(candidates, piece, board, occupancy, en_passant_square);
  for (auto it = candidates.begin(); it != candidates.end(); ) {
    if (!it->matches_algebraic(source_file, source_rank, is_capture, target)) {
      it = candidates.erase(it);
    } else {
      it++;
    }
  }
  return candidates;
}

void State::make_moves(std::string algebraic_variation) {
  boost::regex algebraic_separator("\\s+(\\d+\\.)?");
  std::vector<std::string> algebraic_moves;
  boost::algorithm::split_regex(algebraic_moves,
                                boost::algorithm::trim_copy(algebraic_variation),
                                algebraic_separator);
  make_moves(algebraic_moves);
}

void State::make_moves(std::vector<std::string> algebraic_moves) {
  for (std::string algebraic_move: algebraic_moves) {
    make_move(parse_algebraic(algebraic_move));
  }
}

Piece State::moving_piece(const Move& move, const Halfboard& us) const {
  const Bitboard source = squares::partition[move.from()].bitboard;
  for (Piece piece: pieces::values) {
    if (us[piece] & source)
      return piece;
  }
  assert(false);
}

void State::make_move_on_occupancy(const Move& move,
                                   const Square& source, const Square& target,
                                   Occupancy& occupancy) const {
  occupancy[us] &= ~source;
  occupancy[us] |=  target;

  switch (move.type()) {
  case Move::Type::capture:
    if (target == en_passant_square)
      occupancy[them] &= ~(target >> directions::vertical);
    else
      occupancy[them] &= ~target;
    break;
  case Move::Type::double_push:
  case Move::Type::castle_kingside:
  case Move::Type::castle_queenside:
  case Move::Type::promotion_knight:
  case Move::Type::promotion_bishop:
  case Move::Type::promotion_rook:
  case Move::Type::promotion_queen:
  case Move::Type::normal:
    break;
  default:
    throw std::runtime_error("unhandled Move::Type case");
  }
}

void State::make_move_on_their_halfboard(const Move& move, const Piece piece,
                                         const Square& source, const Square& target,
                                         Halfboard& their_halfboard) const {
  switch (move.type()) {
  case Move::Type::capture:
    if (target == en_passant_square) {
      assert(piece == pieces::pawn);
      // the captured pawn is south from target
      their_halfboard[pieces::pawn] &= ~(target >> directions::vertical);
    } else {
      for (Piece capturee: pieces::values)
        their_halfboard[capturee] &= ~target;
    }
    break;
  case Move::Type::double_push:
  case Move::Type::castle_kingside:
  case Move::Type::castle_queenside:
  case Move::Type::promotion_knight:
  case Move::Type::promotion_bishop:
  case Move::Type::promotion_rook:
  case Move::Type::promotion_queen:
  case Move::Type::normal:
    break;
  default:
    throw std::runtime_error("unhandled Move::Type case");
  }
}

void State::make_move_on_our_halfboard(const Move& move, const Piece piece,
                                       const Square& source, const Square& target,
                                       Halfboard& our_halfboard) const {
  our_halfboard[piece] &= ~source;
  our_halfboard[piece] |= target;

  using namespace pieces;

  switch (move.type()) {
  case Move::Type::castle_kingside:
    our_halfboard[rook] &= ~squares::h1;
    our_halfboard[rook] |=  squares::f1;
    break;
  case Move::Type::castle_queenside:
    our_halfboard[rook] &= ~squares::a1;
    our_halfboard[rook] |=  squares::d1;
    break;
  case Move::Type::promotion_knight:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target;
    our_halfboard[knight] |= target;
    break;
  case Move::Type::promotion_bishop:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target;
    our_halfboard[bishop] |= target;
    break;
  case Move::Type::promotion_rook:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target;
    our_halfboard[rook] |= target;
    break;
  case Move::Type::promotion_queen:
    assert(piece == pawn);
    our_halfboard[piece] &= ~target;
    our_halfboard[queen] |= target;
    break;
  case Move::Type::capture:
  case Move::Type::double_push:
  case Move::Type::normal:
    break;
  default:
    throw std::runtime_error("unhandled Move::Type case");
  }
}

void State::update_castling_rights(const Move& move, const Piece piece,
                                   const Square& source, const Square& target,
                                   bool& can_castle_kingside,
                                   bool& can_castle_queenside) const {
  if (piece == pieces::king) {
    can_castle_kingside = false;
    can_castle_queenside = false;
  } else if (piece == pieces::rook) {
    if (source == squares::h1)
      can_castle_kingside = false;
    else if (source == squares::a1)
      can_castle_queenside = false;
  }
}

void State::update_en_passant_square(const Move& move, const Piece piece,
                                     const Square& source, const Square& target,
                                     Bitboard& en_passant_square) const {
  switch (move.type()) {
  case Move::Type::double_push:
    assert(piece == pieces::pawn);
    en_passant_square = target;
    break;
  case Move::Type::capture:
  case Move::Type::castle_kingside:
  case Move::Type::castle_queenside:
  case Move::Type::promotion_knight:
  case Move::Type::promotion_bishop:
  case Move::Type::promotion_rook:
  case Move::Type::promotion_queen:
  case Move::Type::normal:
    en_passant_square = 0;
    break;
  default:
    throw std::runtime_error("unhandled Move::Type case");
  }
}

void State::make_move(const Move& move) {
  Square source = squares::partition[move.from()],
         target = squares::partition[move.to()];
  Halfboard& our_halfboard = board[us];
  Piece piece = moving_piece(move, our_halfboard);

  update_castling_rights(move, piece, source, target, can_castle_kingside[us], can_castle_queenside[us]);
  make_move_on_our_halfboard(move, piece, source, target, board[us]);
  make_move_on_their_halfboard(move, piece, source, target, board[them]);
  make_move_on_occupancy(move, source, target, occupancy);
  update_en_passant_square(move, piece, source, target, en_passant_square);

  compute_attacks();
  flip_perspective();
}

void State::compute_occupancy() {
  board::flatten(board, occupancy);
}

// NOTE: computes our attacks and assigns to their_attacks; caller should flip_perspective() after this
void State::compute_attacks() {
  Bitboard flat_occupancy;
  board::flatten(occupancy, flat_occupancy);
  their_attacks = moves::all_attacks(flat_occupancy, board[us]);
}

void State::flip_perspective() {
  board::flip_vertically(board);
  board::flip_vertically(en_passant_square);
  board::flip_vertically(occupancy);
  board::flip_vertically(their_attacks);

  std::swap(us, them);
}
