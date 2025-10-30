#include <cstdint>
#include <vector>
#include "Util.h"

extern "C" {
int64_t test3(int64_t* array1, int64_t length);
int64_t refs();
}

// Test nested while loops and reference semantics
int main() {
  std::vector<int64_t> array1 = {1, 2, 3, 4, 5};

  // Test nested loops with array access
  run_test(150, test3(array1.data(), array1.size()), "test3 - nested loops");

  // Test array reference assignment
  run_test(100, refs(), "refs - array reference");
}
