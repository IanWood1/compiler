#include <iostream>
static int total_tests = 0;
static int total_passed = 0;

template <class Result, class Expected>
bool run_test(Expected expected, Result result, const std::string& test_name) {
  total_tests++;
  std::cout << "Testing function: " << test_name << std::endl;
  std::cout << "\tExpected: " << expected << " result: " << result << std::endl;
  if (result != expected) {
    std::cout << "\tFAIL" << std::endl;
    exit(1);
  }
  std::cout << "\tPASS!" << std::endl;
  total_passed++;
  return true;
}

template <class Input>
void check_const_input(Input val1, Input val2) {
  if (val1 != val2) {
    std::cout << "memory corrupted, input should have been immutable";
    exit(1);
  }
  std::cout << "const check passed\n";
}
