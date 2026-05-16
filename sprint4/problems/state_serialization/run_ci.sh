#!/bin/bash
set -e

# Переходим в директорию с Dockerfile
cd "$(dirname "$0")"

echo "=== Building Docker image ==="
docker build -t state_serialization:latest .

echo "=== Running tests ==="
docker run --rm \
  -v /var/run/docker.sock:/var/run/docker.sock \
  state_serialization:latest -s

echo "=== Tests completed successfully ==="
