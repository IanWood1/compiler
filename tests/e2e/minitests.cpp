#include <cstdint>
#include <iostream>
#include "Util.h"

extern "C" {
int64_t minitest1();
int64_t minitest2();
int64_t minitest3();
int64_t minitest4();
int64_t minitest5();
}

// Collection of smaller tests for various language features
int main() {
  run_test(13, minitest1(), "minitest1 - forward constant");
  run_test(115, minitest2(), "minitest2 - reference array modification");
  run_test(15, minitest3(), "minitest3 - array sum with loop");
  run_test(10, minitest4(), "minitest4 - return array by value");
  run_test(0, minitest5(), "minitest5 - value vs reference semantics");

  std::cout << "\nPassed " << total_passed << " of " << total_tests
            << " tests\n";
  return 0;
}
