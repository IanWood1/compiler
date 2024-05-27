#!/bin/bash
# Script used to run tests.

echo "Building test deps"
cmake --build build --target compiler_tests 
echo "Starting tests..."
cmake --build build --target test

RESULT=$?
echo "Finished running tests..."
exit $RESULT
