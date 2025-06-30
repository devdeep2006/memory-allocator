#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Running Benchmarks ---"

# Build benchmarks first
./scripts/build.sh

# Run the benchmark executable that compares custom vs standard malloc
echo "Running custom allocator benchmark..."
./bin/benchmark

echo "Running comparison with standard malloc..."
./bin/compare_malloc

echo "All benchmarks completed."