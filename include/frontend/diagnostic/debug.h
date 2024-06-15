// DEBUG macros for error handling and debugging
#pragma once
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

extern bool enableDebug;

#define FRONTEND_ERROR(msg)                                            \
  std::cerr << "FRONTEND_ERROR: " << (msg) << " [in file " << __FILE__ \
            << " on line " << __LINE__ << "]" << std::endl;            \
  exit(1)

#define NOT_IMPLEMENTED()            \
  FRONTEND_ERROR("not implemented"); \
  (void*)0

#ifdef DEBUGIR

#define DEBUG_PRINT(msg)                                                \
  do {                                                                  \
    if (enableDebug) {                                                  \
      std::cout << "DEBUG PRINT: " << (msg) << " [in file " << __FILE__ \
                << " on line " << __LINE__ << "]" << std::endl          \
    }                                                                   \
    while (0)

#define DEBUG_PRINT_VECTOR(vec) \
  std::cout << #vec << ": ";    \
  for (auto i : (vec)) {        \
    std::cout << i << " ";      \
  }                             \
  std::cout << std::endl
#else
#define ASSERT(condition, msg)
#define DEBUG_PRINT(msg)
#define DEBUG_PRINT_VECTOR(vec)
#endif  // DEBUGIR
