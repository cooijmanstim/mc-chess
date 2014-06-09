// ideally cheap to construct, possibly expensive to apply
struct Undo {
  Move move;
  size_t prior_halfmove_clock;
  Bitboard prior_en_passant_square;
  CastlingRights prior_castling_rights;
  Piece captured_piece;
  Bitboard capture_square;

  inline void record_capture(Piece piece, Bitboard square) {
    captured_piece = piece;
    capture_square = square;
  }

  inline void record_no_capture() {
    captured_piece = pieces::pawn;
    capture_square = 0;
  }
};
