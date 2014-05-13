#include "colors.hpp"

std::string colors::name(Color color) {
  return color == white ? "white" : "black";
}

char colors::symbol(Color color) {
  return name(color)[0];
}
