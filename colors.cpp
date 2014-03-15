#include "colors.hpp"

std::string colors::name(Color color) {
  return color == white ? "white" : "black";
}
