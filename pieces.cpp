#include <boost/format.hpp>

#include "pieces.hpp"

std::map<std::string, Piece> by_name = {
  {"",       pieces::pawn},
  {"n",      pieces::knight},
  {"b",      pieces::bishop},
  {"r",      pieces::rook},
  {"q",      pieces::queen},
  {"k",      pieces::king},
  {"N",      pieces::knight},
  {"B",      pieces::bishop},
  {"R",      pieces::rook},
  {"Q",      pieces::queen},
  {"K",      pieces::king},
  {"pawn",   pieces::pawn},
  {"knight", pieces::knight},
  {"bishop", pieces::bishop},
  {"rook",   pieces::rook},
  {"queen",  pieces::queen},
  {"king",   pieces::king},
};

Piece pieces::type_from_name(std::string name) {
  try {
    return by_name[name];
  } catch (std::out_of_range& e) {
    throw std::invalid_argument(str(boost::format("unknown piece name: %1%") % name));
  }
}
