#include <iostream>

namespace util {

// Global flag to control debugging
static bool debug_enabled = false;

// Function to enable debugging
static void enable_debug(bool enable) {
  debug_enabled = enable;
}

static std::ostream& debug() {
  static std::ostream null_stream(nullptr);
  static bool debug_enabled = false;

  if (debug_enabled) {
    return std::cout;
  } else {
    return null_stream;
  }
}

}  // namespace util
