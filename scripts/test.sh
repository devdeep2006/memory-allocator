#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Running Tests ---"

# Build tests first to ensure they are up-to-date
./scripts/build.sh

# Run make test (Makefile target will execute all test binaries)
make test

echo "All tests completed."