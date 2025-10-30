#include <cstdint>
#include <vector>
#include "Util.h"

extern "C" {
int64_t test4(int64_t* array1, int64_t* array2);
}

// Test array forwarding and reference vs value semantics
// Tests functions that return arrays by value and by reference
int main() {
  std::vector<int64_t> array1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<int64_t> array2 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  
  // test4 returns 0 when arrays are the same
  run_test(0, test4(array1.data(), array2.data()), "test4 - same arrays");
  
  // Check that arrays are not modified (pass by value semantics)
  check_const_input(array1, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  
  // Test with different arrays (returns 2 because test4_compare is called twice)
  std::vector<int64_t> array3 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
  run_test(2, test4(array1.data(), array3.data()), "test4 - different arrays");
  
  return 0;
}

