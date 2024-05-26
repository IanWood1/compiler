#!/bin/bash
# Script used to build CMake Release files.
echo "Started building Release CMAKE files..."

# Clearing build folder if it exists
rm -r build/ &> /dev/null

cmake -B build
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

echo "Bulding Release version using ${cpu_count} threads..."
cmake --build build/ -j$cpu_count
RESULT=$?
echo "Finished building Release!"
exit $RESULT

