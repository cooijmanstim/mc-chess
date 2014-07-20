#pragma once

#include <fstream>

#include <boost/random.hpp>

#include "prettyprint.hpp"

// TODO: solve this moare betterlye
template <typename T, int M, int N> using array2d = std::array<std::array<T, N>, M>;
template <typename T, int M, int N, int O> using array3d = std::array<std::array<std::array<T, O>, N>, M>;

void print_backtrace();

void sleep_forever();

#include <vector>
class State;
class Move;
void dump_for_debug(State state, std::vector<Move> moves);

std::vector<std::string> words(std::string string);

template <typename T>
T random_element(std::vector<T> const& elements, boost::mt19937& generator) {
  boost::uniform_int<> distribution(0, elements.size() - 1);
  return elements.at(distribution(generator));
}

extern boost::normal_distribution<double> standard_normal_distribution;

// serialization for std::array
#include <boost/serialization/array.hpp>
namespace boost {
  namespace serialization {
    template<class Archive, class T, size_t N>
    void serialize(Archive & ar, std::array<T,N> & a, const unsigned int version)
    {
      ar & boost::serialization::make_array(a.data(), a.size());
    }
  }
}

inline bool file_readable(std::string path) {
  std::ifstream infile(path);
  return infile.good();
}
