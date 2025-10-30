#include <cstdint>
#include <vector>
#include <iostream>
#include "Util.h"

extern "C" {
int64_t testArrayInit();
int64_t testArrayBounds(int64_t* arr);
int64_t testArrayAssign();
int64_t testArrayCopy();
int64_t testUniformArray();
int64_t testRefAssignment();
int64_t testNestedArrayOps(int64_t* arr1, int64_t* arr2);
int64_t testArrayElemAsParam();
int64_t testSequential();
int64_t testLargeArray();
int64_t testMultipleAccess();
}

// Test array edge cases: initialization, bounds, references, copying
int main() {
  // Test array initialization with literal
  run_test(84, testArrayInit(), "testArrayInit - [42;5]");
  
  // Test array boundary access
  std::vector<int64_t> arr1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  run_test(11, testArrayBounds(arr1.data()), "testArrayBounds - first + last");
  
  // Test array assignment
  run_test(60, testArrayAssign(), "testArrayAssign - 10+20+30");
  
  // Test array copy (should be 1, not 99, if copy semantics work)
  run_test(1, testArrayCopy(), "testArrayCopy - value semantics");
  
  // Test uniform array: sum of [7;10] = 70
  run_test(70, testUniformArray(), "testUniformArray - sum");
  
  // Test reference assignment (should be 99 after modification)
  run_test(99, testRefAssignment(), "testRefAssignment - ref modifies original");
  
  // Test nested array operations
  std::vector<int64_t> arr2 = {1, 2, 3, 4, 5};
  std::vector<int64_t> arr3 = {5, 4, 3, 2, 1};
  run_test(12, testNestedArrayOps(arr2.data(), arr3.data()), "testNestedArrayOps - element-wise add");
  
  // Test array element as parameter
  run_test(10, testArrayElemAsParam(), "testArrayElemAsParam - helper(5)");
  
  // Test sequential values: 0+1+2+3+4 = 10
  run_test(10, testSequential(), "testSequential - sum 0..4");
  
  // Test large array
  run_test(3, testLargeArray(), "testLargeArray - 100 elements");
  
  // Test multiple accesses
  run_test(20, testMultipleAccess(), "testMultipleAccess - first + last");
  
  std::cout << "\nPassed " << total_passed << " of " << total_tests
            << " tests\n";
  return 0;
}

