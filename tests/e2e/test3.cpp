#include <cstdint>
#include <vector>
#include "Util.h"

extern "C" int64_t test3(int64_t* array1, int64_t length);
int main() {
  std::vector<int64_t> array1 = {1, 2, 3, 4, 5};

  run_test(150, test3(array1.data(), array1.size()), "test3");
}
