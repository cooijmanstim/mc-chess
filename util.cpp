#include "util.hpp"

#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>

void print_backtrace() {
  const size_t max_size = 30;
  void* array[max_size];
  size_t size;

  size = backtrace(array, max_size);
  fprintf(stderr, "Backtrace:\n");
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}
