#pragma once

#include "prettyprint.hpp"

template <typename T, int M, int N> using array2d = std::array<std::array<T, N>, M>;

void print_backtrace();
