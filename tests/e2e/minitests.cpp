#include <cstdint>
#include <iostream>
#include "Util.h"

extern "C" {
// int64_t test4(int64_t* array1, int64_t* array2);
int64_t minitest1();
int64_t minitest2();
int64_t minitest3();
int64_t minitest4();
}
int main() {
  // std::vector<int64_t> array1 = {1, 2, 3, 4, 5};
  // std::vector<int64_t> array2 = {1, 2, 3, 4, 5};

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
  return 0;
}
