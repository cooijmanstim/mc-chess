#include "state.hpp"
#include "squares.hpp"

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

  board[white][pawn] = a2 | b2 | c2 | d2 | e2 | f2 | g2 | h2;
  board[black][pawn] = a7 | b7 | c7 | d7 | e7 | f7 | g7 | h7;
  board[white][knight] = b1 | g1;
  board[black][knight] = b8 | g8;
  board[white][bishop] = c1 | f1;
  board[black][bishop] = c8 | f8;
  board[white][rook] = a1 | h1;
  board[black][rook] = a8 | h8;
  board[white][queen] = d1;
  board[black][queen] = d8;
  board[white][king] = e1;
  board[black][king] = e8;

  can_castle_kingside.fill(true);
  can_castle_queenside.fill(true);

  color_to_move = colors::white;
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

  for (size_t i = 0; i < squares::cardinality; i++) {
    // bit index with rank flipped
    size_t j = ((7 - (i >> 3)) << 3) + (i & ((1<<3)-1));
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
  }

  o << std::endl;

  o << colors::name(s.color_to_move) << " to move. ";

  o << " castling rights: ";
  if (s.can_castle_kingside [colors::white]) o << "k";
  if (s.can_castle_queenside[colors::white]) o << "q";
  if (s.can_castle_kingside [colors::black]) o << "K";
  if (s.can_castle_queenside[colors::black]) o << "Q";

  if (s.en_passant_square)
    o << " en-passant square: " << squares::name_from_bitboard(*s.en_passant_square);

  o << std::endl;
  return o;
}
