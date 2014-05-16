#include "notation.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>

Move notation::algebraic::parse(std::string string, const State& state) {
  static boost::regex regex("(([NBRQK]?)([a-h])?([1-8])?(x)?([a-h][1-8])(=([NBRQ]))?|(O-O-O|0-0-0)|(O-O|0-0))[+#]?");
  boost::smatch m;
  if (!boost::regex_match(string, m, regex))
    throw std::runtime_error(str(boost::format("can't parse algebraic move: %1%") % string));

  std::function<bool(const Move&)> predicate;
  if (m[9].matched) {
    predicate = [&](const Move& move) {
      return move == Move::castle(state.us, castles::queenside);
    };
  } else if (m[10].matched) {
    predicate = [&](const Move& move) {
      return move == Move::castle(state.us, castles::kingside);
    };
  } else {
    Piece piece = pieces::type_from_name(std::string(m[2].first, m[2].second));

    boost::optional<files::Index> source_file;
    boost::optional<ranks::Index> source_rank;
    if (m[3].matched) source_file = files::by_keyword(std::string(m[3].first, m[3].second));
    if (m[4].matched) source_rank = ranks::by_keyword(std::string(m[4].first, m[4].second));

    bool is_capture = m[5].matched;

    squares::Index target = squares::by_keyword(std::string(m[6].first, m[6].second));

    boost::optional<Piece> promotion;
    if (m[7].matched) promotion = pieces::type_from_name(std::string(m[8].first, m[8].second));

    predicate = [=](const Move& move) {
      auto source_piece = state.colored_piece_at(move.source());
      if (!source_piece || *source_piece != ColoredPiece(state.us, piece))
        return false;
      if (source_file && *source_file != files::by_square(move.source()))
        return false;
      if (source_rank && *source_rank != ranks::by_square(move.source()))
        return false;
      if (is_capture != move.is_capture())
        return false;
      if (promotion != move.promotion())
        return false;
      if (target != move.target())
        return false;
      return true;
    };
  }

  std::vector<Move> candidates = state.moves();

  for (auto it = candidates.begin(); it != candidates.end(); ) {
    if (!predicate(*it)) {
      it = candidates.erase(it);
    } else {
      it++;
    }
  }

  if (candidates.empty())
    throw OverdeterminedException(str(boost::format("no match for algebraic move: %1%") % string));
  if (candidates.size() > 1)
    throw UnderdeterminedException(str(boost::format("ambiguous algebraic move: %1%, candidates: %2%") % string % candidates));
  return candidates[0];
}

Move notation::coordinate::parse(std::string string, const State& state) {
  static boost::regex regex("([a-h][1-8])([a-h][1-8])([kbrq])?");
  boost::smatch m;
  if (!boost::regex_match(string, m, regex))
    throw std::runtime_error(str(boost::format("can't parse algebraic move: %1%") % string));
  squares::Index source = squares::by_keyword(std::string(m[1].first, m[1].second));
  squares::Index target = squares::by_keyword(std::string(m[2].first, m[2].second));
  boost::optional<Piece> promotion;
  if (m[3].matched)
    promotion = pieces::type_from_name(std::string(m[3].first, m[3].second));

  for (Move move: state.moves()) {
    if (move.source() != source)
      continue;
    if (move.target() != target)
      continue;
    if (move.promotion() != promotion)
      continue;
    return move;
  }

  std::cerr << "no match for coordinate move " << string << " in state:" << std::endl;
  std::cerr << state << std::endl;
  throw std::runtime_error(str(boost::format("no match for coordinate move: %1%") % string));
}

std::string notation::coordinate::format(const Move& move) {
  boost::optional<Piece> promotion = move.promotion();
  return str(boost::format("%1%%2%%3%")
    % squares::keywords[move.source()]
    % squares::keywords[move.target()]
    % (promotion
       ? std::string(1, pieces::symbols[*promotion])
       : ""));
}
