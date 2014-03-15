#include "state.hpp"

State::State() :
  color_to_move(Color::White)
{
  can_castle_kingside.fill(true);
  can_castle_queenside.fill(true);
  
  for (size_t i = 0; i < colors::cardinality; i++) {
    for (size_t j = 0; j < pieces::cardinality; j++) {
      board[i][j] = 0;
    }
  }
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
