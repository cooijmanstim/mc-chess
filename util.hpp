#pragma once

#include "prettyprint.hpp"

// TODO: solve this moare betterlye
template <typename T, int M, int N> using array2d = std::array<std::array<T, N>, M>;
template <typename T, int M, int N, int O> using array3d = std::array<std::array<std::array<T, O>, N>, M>;

void print_backtrace();
