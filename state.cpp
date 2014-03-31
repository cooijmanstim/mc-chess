#include <string>
#include <cctype>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/format.hpp>

#include "state.hpp"
#include "direction.hpp"

State::State()
{
  for (size_t i = 0; i < colors::cardinality; i++) {
    for (size_t j = 0; j < pieces::cardinality; j++) {
      board[i][j] = 0;
    }
  }

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

  color_to_move = colors::white;

  en_passant_square = 0;
}

State::State(std::string fen)
{
  boost::regex fen_regex("((\\w+/){7}\\w+)\\s+([bw])\\s+(K)?(Q)?(k)?(q)?\\s+(([a-h][1-8])|-)\\s+.*");
  boost::smatch m;
  if (!boost::regex_match(fen, m, fen_regex))
    throw std::runtime_error(str(boost::format("can't parse FEN: %1%") % fen));

  std::string board(m[1].first, m[1].second);

  boost::regex rank_separator("/");
  std::vector<std::string> fen_ranks;
  boost::algorithm::split_regex(fen_ranks, std::string(m[1].first, m[1].second), rank_separator);
  std::reverse(fen_ranks.begin(), fen_ranks.end());

  using namespace pieces;
  squares::Index square_index = 0;
  for (BoardPartition::Part rank: ranks::partition) {
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

  color_to_move = std::string(m[3].first, m[3].second) == "w" ? colors::white : colors::black;

  can_castle_kingside  = {m[4].matched, m[6].matched};
  can_castle_queenside = {m[5].matched, m[7].matched};

  if (m[9].matched)
    en_passant_square = squares::partition[std::string(m[9].first, m[9].second)];
}

State::State(State &that) :
  board(that.board),
  can_castle_kingside(that.can_castle_kingside),
  can_castle_queenside(that.can_castle_queenside),
  en_passant_square(that.en_passant_square),
  color_to_move(that.color_to_move)
{
}

State::~State() {}

bool State::operator==(State &that) {
  if (this == &that)
    return true;
  
  return this->color_to_move == that.color_to_move &&
         this->en_passant_square == that.en_passant_square &&
         this->can_castle_kingside == that.can_castle_kingside &&
         this->can_castle_queenside == that.can_castle_queenside &&
         this->board == that.board;
}

std::ostream& operator<<(std::ostream& o, const State& s) {
  array2d<char, colors::cardinality, pieces::cardinality> symbols = {
    'p', 'n', 'b', 'r', 'q', 'k',
    'P', 'N', 'B', 'R', 'Q', 'K',
  };

  for (squares::Index i: squares::partition.indices) {
    o << ' ';

    // bit index with rank flipped
    size_t j = ((7 - (i >> 3)) << 3) + (i & 7);

    Bitboard square = Bitboard(1)<<j;
    bool piece_found = false;
    for (Color c: colors::values) {
      for (Piece p: pieces::values) {
        if (s.board[c][p] & square) {
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

  o << colors::name(s.color_to_move) << " to move. ";

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

std::vector<Move> State::moves() const {
  // NOTE: move generation always generates from white's perspective. modify board accordingly.

  // TODO: maybe reserve()
  std::vector<Move> moves;
  moves::all_moves(moves, board, occupancy(), en_passant_square);
  return moves;
}

Move State::parse_algebraic(std::string algebraic) const {
  boost::regex algebraic_move_regex("([NBRQK]?)([a-h])?([1-8])?(x)?([a-h][1-8])+?");
  boost::smatch m;
  if (!boost::regex_match(algebraic, m, algebraic_move_regex))
    throw std::runtime_error(str(boost::format("can't parse algebraic move: %1%") % algebraic));

  Piece piece = pieces::type_from_name(std::string(m[1].first, m[1].second));

  boost::optional<BoardPartition::Part> source_file;
  boost::optional<BoardPartition::Part> source_rank;
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

// TODO: maybe keep track of as we go along. maybe.
Occupancy State::occupancy() const {
  Occupancy occupancy;
  for (Color c: colors::values) {
    occupancy[c] = 0;
    for (Piece p: pieces::values)
      occupancy[c] |= board[c][p];
  }
  return occupancy;
}

std::vector<Move> State::match_algebraic(Piece piece,
                                         boost::optional<BoardPartition::Part> source_file,
                                         boost::optional<BoardPartition::Part> source_rank,
                                         bool is_capture,
                                         squares::Index target) const {
  std::vector<Move> candidates;
  moves::piece_moves(candidates, piece, board, occupancy(), en_passant_square);

  std::remove_if(candidates.begin(), candidates.end(),
                 [source_file, source_rank, is_capture, target](const Move& candidate) {
      if (source_file && !(*source_file & squares::partition[candidate.from()]))
        return true;
      if (source_rank && !(*source_rank & squares::partition[candidate.from()]))
        return true;
      if (is_capture && candidate.is_capture())
        return true;
      if (candidate.to() != target)
        return true;
      return false;
    });

  return candidates;
}

void State::make_moves(std::string algebraic_variation) {
  // TODO: figure out how to deal with monochromeness 'n' stuff
  boost::regex algebraic_separator("\\s+(\\d+\\.)?");
  std::vector<std::string> algebraic_moves;
  boost::algorithm::split_regex(algebraic_moves,
                                boost::algorithm::trim_copy(algebraic_variation),
                                algebraic_separator);
  make_moves(algebraic_moves);
}

void State::make_moves(std::vector<std::string> algebraic_moves) {
  for (std::string algebraic_move: algebraic_moves) {
    // TODO: color
    make_move(parse_algebraic(algebraic_move));
  }
}

// TODO: overload with second argument for returning undo information
void State::make_move(Move m) {
  Bitboard source = squares::partition[m.from()].bitboard,
           target = squares::partition[m.to()].bitboard;

  std::array<Bitboard, pieces::cardinality> us = board[colors::white], them = board[colors::black];
  for (Piece piece: pieces::values) {
    if (!(us[piece] & source))
      continue;

    us[piece] &= ~source | target;

    if (piece == pieces::king) {
      can_castle_kingside[colors::white] = false;
      can_castle_queenside[colors::white] = false;
    } else if (piece == pieces::rook) {
      if (source == squares::h1)
        can_castle_kingside[colors::white] = false;
      else if (source == squares::a1)
        can_castle_queenside[colors::white] = false;
    }

    switch (m.type()) {
    case Move::Type::capture:
      if (target == en_passant_square) {
        assert(piece == pieces::pawn);
        // the captured pawn is south from target
        them[piece] &= ~(target >> directions::vertical);
      } else {
        for (Piece capturee: pieces::values)
          them[capturee] &= ~target;
      }
      break;
    case Move::Type::double_push:
      en_passant_square = target;
      break;
    case Move::Type::castle_kingside:
      us[pieces::rook] &= ~squares::h1 | squares::f1;
      break;
    case Move::Type::castle_queenside:
      us[pieces::rook] &= ~squares::a1 | squares::d1;
      break;
    case Move::Type::promotion_knight:
      assert(piece == pieces::pawn);
      us[piece] &= ~target;
      us[pieces::knight] |= target;
      break;
    case Move::Type::promotion_bishop:
      assert(piece == pieces::pawn);
      us[piece] &= ~target;
      us[pieces::bishop] |= target;
      break;
    case Move::Type::promotion_rook:
      assert(piece == pieces::pawn);
      us[piece] &= ~target;
      us[pieces::rook] |= target;
      break;
    case Move::Type::promotion_queen:
      assert(piece == pieces::pawn);
      us[piece] &= ~target;
      us[pieces::queen] |= target;
      break;
    case Move::Type::normal:
      break;
    default:
      throw std::runtime_error("unhandled Move::Type case");
    }
  }

  // TODO: if king is in check now, lose
  // TODO: flip board vertically a la monochromicity
  // TODO: unset en_passant_square if not double push
}
