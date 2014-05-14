#pragma once

#include "move.hpp"
#include "state.hpp"

namespace notation {
  namespace algebraic {
    Move parse(std::string string, const State& state);

    class UnderdeterminedException : public std::runtime_error {
    public:
      UnderdeterminedException(std::string message)
        : std::runtime_error(message)
      {}
    };
    class OverdeterminedException : public std::runtime_error {
    public:
      OverdeterminedException(std::string message)
        : std::runtime_error(message)
      {}
    };
  }

  namespace coordinate {
    Move parse(std::string string, const State& state);
    std::string format(const Move& move);
  }
}
