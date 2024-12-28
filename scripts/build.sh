#!/bin/bash

set -euxo pipefail

# Script used to build CMake Release files.
echo "Started building Release CMAKE files..."

sudo apt install llvm-15-dev

# Clearing build folder if it exists
rm -r build/ &>/dev/null

cmake -B build -G Ninja
RESULT=$?
if [ "$RESULT" -ne 0 ]; then
  exit $RESULT
fi

echo "Finished building Release CMAKE files!"

if [ "$(uname -s)" == "Linux" ]; then
  cpu_count=$(nproc)
elif [ "$(uname -s)" == "Darwin" ]; then
  cpu_count=$(sysctl -n hw.ncpu)
else
  exit 1
fi

echo "Bulding Release version..."
cmake --build build/
RESULT=$?
echo "Finished building Release!"
exit $RESULT
