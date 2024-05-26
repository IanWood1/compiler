#include <cstdint>
#include <iostream>
#include <vector>

extern "C" {
int64_t test1(int64_t* array1, int64_t* array2, int64_t length);
int64_t test2(int64_t v1, int64_t v2);
int64_t test3(int64_t* array1, int64_t length);
int64_t test4(int64_t* array1, int64_t* array2);
int64_t minitest1();
int64_t minitest2();
int64_t minitest3();
int64_t minitest4();
}

static int total_tests = 0;
static int total_passed = 0;

template <class Result, class Expected>
bool run_test(Expected expected, Result result, const std::string& test_name) {
  total_tests++;
  std::cout << "Testing function: " << test_name << std::endl;
  std::cout << "\tExpected: " << expected << " result: " << result << std::endl;
  if (result != expected) {
    std::cout << "\tFAIL" << std::endl;
    return false;
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

int main() {
  std::vector<int64_t> array1 = {1, 2, 3, 4, 5};
  std::vector<int64_t> array2 = {1, 2, 3, 4, 5};
  run_test(30, test1(array1.data(), array2.data(), array1.size()), "test1");
  check_const_input(array1, {1, 2, 3, 4, 5});
  check_const_input(array2, {1, 2, 3, 4, 5});

  run_test(3, test2(1, 2), "test2");

  array1 = {1, 2, 3, 4, 5};
  run_test(150, test3(array1.data(), array1.size()), "test3");
  check_const_input(array1, {1, 2, 3, 4, 5});

  //  array1 = {1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
  //  array2 = {1, 2, 3, 4, 5, 1, 2, 3, 4, 5};
  //  run_test(0, test4(array1.data(), array2.data()), "test4");
  //  check_const_input(array1, {1, 2, 3, 4, 5, 1, 2, 3, 4, 5});

  run_test(13, minitest1(), "minitest1");
  run_test(115, minitest2(), "minitest2");
  run_test(15, minitest3(), "minitest3");
  run_test(10, minitest4(), "minitest4");

  std::cout << "\nPassed " << total_passed << " of " << total_tests
            << " tests\n";
}
