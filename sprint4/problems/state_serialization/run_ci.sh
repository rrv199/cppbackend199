#!/bin/bash
set -e

echo "=== Building Docker image ==="
docker build -t state_serialization:latest .

echo "=== Running tests ==="
docker run --rm state_serialization:latest -s

echo "=== Tests completed ==="
