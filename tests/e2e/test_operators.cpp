#include <cstdint>
#include <iostream>
#include <vector>
#include "Util.h"

extern "C" {
int64_t testArithmetic(int64_t a, int64_t b);
int64_t testShiftLeft(int64_t x, int64_t shift);
int64_t testShiftRight(int64_t x, int64_t shift);
int64_t testLessThan(int64_t a, int64_t b);
int64_t testLessEqual(int64_t a, int64_t b);
int64_t testGreaterThan(int64_t a, int64_t b);
int64_t testGreaterEqual(int64_t a, int64_t b);
int64_t testEqual(int64_t a, int64_t b);
int64_t testComplexExpr(int64_t a, int64_t b, int64_t c);
int64_t testPrecedence(int64_t x);
int64_t testChainedOps(int64_t a, int64_t b, int64_t c);
int64_t testArrayOps(int64_t* arr);
int64_t testShiftOps(int64_t base);
int64_t testZeroOps(int64_t x);
int64_t testSubtraction(int64_t a);
}

// Test arithmetic, bitwise, and comparison operators
int main() {
  // Test arithmetic: (5+3) + (5-3) + (5*3) = 8 + 2 + 15 = 25
  run_test(25, testArithmetic(5, 3), "testArithmetic");

  // Test shift operations
  run_test(32, testShiftLeft(8, 2), "testShiftLeft - 8 << 2");
  run_test(4, testShiftRight(16, 2), "testShiftRight - 16 >> 2");
  run_test(2, testShiftRight(5, 1), "testShiftRight - 5 >> 1");

  // Test comparison operators (return 0 for false, 1 for true)
  run_test(1, testLessThan(5, 10), "testLessThan - true");
  run_test(0, testLessThan(10, 5), "testLessThan - false");

  run_test(1, testLessEqual(5, 5), "testLessEqual - equal");
  run_test(1, testLessEqual(5, 10), "testLessEqual - less");

  run_test(1, testGreaterThan(10, 5), "testGreaterThan - true");
  run_test(0, testGreaterThan(5, 10), "testGreaterThan - false");

  run_test(1, testGreaterEqual(5, 5), "testGreaterEqual - equal");
  run_test(1, testGreaterEqual(10, 5), "testGreaterEqual - greater");

  run_test(1, testEqual(7, 7), "testEqual - true");
  run_test(0, testEqual(7, 8), "testEqual - false");

  // Test complex expressions: a + b*c - a = 2 + 3*4 - 2 = 12
  run_test(12, testComplexExpr(2, 3, 4), "testComplexExpr");

  // Test operator precedence: 5 + 5*5 = 30
  run_test(30, testPrecedence(5), "testPrecedence");

  // Test chained comparisons
  run_test(2, testChainedOps(1, 5, 10), "testChainedOps - both true");
  run_test(1, testChainedOps(1, 10, 5), "testChainedOps - one true");

  // Test array operations: arr[0] + arr[1]*arr[2] - arr[3]
  std::vector<int64_t> arr = {10, 2, 5, 3, 0};
  run_test(17, testArrayOps(arr.data()), "testArrayOps - 10 + 2*5 - 3");

  // Test shift with variables: (8 << 2) + (8 >> 1) = 32 + 4 = 36
  run_test(36, testShiftOps(8), "testShiftOps");

  // Test operations with zero: x + 0 - 0 + x*0 = x
  run_test(5, testZeroOps(5), "testZeroOps");

  // Test subtraction: (a - a) + a = a
  run_test(10, testSubtraction(10), "testSubtraction");

  std::cout << "\nPassed " << total_passed << " of " << total_tests
            << " tests\n";
  return 0;
}
