#!/bin/bash

set -euxo pipefail

echo "Building test deps"
cmake --build build --target compiler_tests
echo "Starting tests..."
cmake --build build --target test

echo "Finished running tests..."
