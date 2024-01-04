// DEBUG macros for error handling and debugging
#pragma once
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#define DEBUGIR
#define FRONTEND_ERROR(msg)                                            \
  std::cerr << "FRONTEND_ERROR: " << (msg) << " [in file " << __FILE__ \
            << " on line " << __LINE__ << "]" << std::endl;            \
  exit(1)

#ifdef DEBUGIR

#define NOT_IMPLEMENTED() FRONTEND_ERROR("not implemented");
#define ASSERT(condition, msg)                                             \
  if (!(condition)) {                                                      \
    std::cerr << "ASSERTION FAILED: " << (msg) << " [in file " << __FILE__ \
              << " on line " << __LINE__ << "]" << std::endl;              \
    exit(1);                                                               \
  }                                                                        \
  (void*)0
#define DEBUG_PRINT(msg)                                            \
  std::cout << "DEBUG PRINT: " << (msg) << " [in file " << __FILE__ \
            << " on line " << __LINE__ << "]" << std::endl
#define DEBUG_PRINT_VECTOR(vec) \
  std::cout << #vec << ": ";    \
  for (auto i : (vec)) {        \
    std::cout << i << " ";      \
  }                             \
  std::cout << std::endl
#else
#define NOT_IMPLEMENTED()
#define ASSERT(condition, msg)
#define DEBUG_PRINT(msg)
#define DEBUG_PRINT_VECTOR(vec)
#endif  // DEBUGIR
