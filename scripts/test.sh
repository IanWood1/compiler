#!/bin/bash
# Script used to run tests.

echo "Starting tests..."
./build/bin/run-tests

RESULT=$?
echo "Finished running tests..."
exit $RESULT
