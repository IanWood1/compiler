#include <cstdint>
#include "Util.h"

extern "C" {
int64_t test2(int64_t, int64_t);
}

// Simple test: basic function call with two integers
int main() {
  run_test(3, test2(1, 2), "test2 - basic addition");
}
