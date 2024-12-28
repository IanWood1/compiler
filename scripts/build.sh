#!/bin/bash

set -euxo pipefail

echo "Started building CMAKE files..."

cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build/
