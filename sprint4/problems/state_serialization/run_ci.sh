#!/bin/bash
set -e

# Путь к директории с заданием
TASK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../sprint4/problems/state_serialization" && pwd)"

echo "Building Docker image..."
cd "$TASK_DIR"
docker build -t state_serialization:latest .

echo "Running tests..."
docker run --rm state_serialization:latest -s

echo "✅ Tests passed!"
