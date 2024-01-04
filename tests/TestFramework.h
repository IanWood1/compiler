#pragma once
#include <iostream>

template <class RetType, class... Args>
void run_test(const char* name, RetType (*func)(Args...), RetType expected,
              Args&... args) {
  std::cout << "Running test: " << name << std::endl;
  bool result = func(args...) == expected;
  std::cout << "\t" << (result ? "PASS" : "FAIL") << std::endl;
}