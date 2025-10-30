#include <cstdint>
#include <vector>
#include <iostream>
#include "Util.h"

extern "C" {
int64_t testIf(int64_t x);
int64_t testNestedIf(int64_t x, int64_t y);
int64_t testIfComplexCond(int64_t x, int64_t y);
int64_t testWhileWithIf(int64_t n);
int64_t testNestedWhile(int64_t n, int64_t m);
int64_t testEarlyReturn(int64_t* arr, int64_t target);
int64_t testComparisons(int64_t a, int64_t b);
}

// Test control flow: if statements, nested loops, conditionals
int main() {
  // Test basic if statement
  run_test(0, testIf(5), "testIf - false condition");
  run_test(1, testIf(15), "testIf - true condition");
  
  // Test nested if
  run_test(0, testNestedIf(5, -3), "testNestedIf - inner false");
  run_test(0, testNestedIf(-5, 3), "testNestedIf - outer false");
  run_test(1, testNestedIf(5, 3), "testNestedIf - both true");
  
  // Test if with complex condition
  run_test(0, testIfComplexCond(3, 4), "testIfComplexCond - false");
  run_test(15, testIfComplexCond(7, 8), "testIfComplexCond - true");
  
  // Test while with if
  run_test(10, testWhileWithIf(10), "testWhileWithIf - sum [0..4]");
  
  // Test nested while
  run_test(12, testNestedWhile(3, 4), "testNestedWhile - 3x4");
  run_test(25, testNestedWhile(5, 5), "testNestedWhile - 5x5");
  
  // Test early return
  std::vector<int64_t> arr = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
  run_test(4, testEarlyReturn(arr.data(), 50), "testEarlyReturn - found");
  run_test(-1, testEarlyReturn(arr.data(), 999), "testEarlyReturn - not found");
  
  // Test comparison operators
  run_test(11, testComparisons(5, 10), "testComparisons - less than");
  run_test(1110, testComparisons(10, 10), "testComparisons - equal");
  run_test(11000, testComparisons(10, 5), "testComparisons - greater than");
  
  std::cout << "\nPassed " << total_passed << " of " << total_tests
            << " tests\n";
  return 0;
}

