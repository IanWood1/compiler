#include <cstdint>
#include <vector>
#include "Util.h"

extern "C" {
int64_t test1(int64_t* array1, int64_t* array2, int64_t length);
}

// Test complex array operations: element-wise addition, swapping, and memory copying
// Verifies that arrays passed by value are not modified
int main() {
  std::vector<int64_t> array1 = {1, 2, 3, 4, 5};
  std::vector<int64_t> array2 = {1, 2, 3, 4, 5};
  int64_t res = test1(array1.data(), array2.data(), array1.size());

  // Ensure input arrays remain unchanged (pass by value semantics)
  check_const_input(array1, {1, 2, 3, 4, 5});
  check_const_input(array2, {1, 2, 3, 4, 5});

  int64_t expected = 30;
  if (res != expected) {
    return 1;
  }
  return 0;
}
