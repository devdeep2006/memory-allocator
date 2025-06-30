#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Building Memory Allocator Project ---"

# Create necessary directories
mkdir -p bin obj

# Run make
make all

echo "Build complete. Executables are in the 'bin/' directory."