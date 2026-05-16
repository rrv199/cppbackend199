#!/bin/bash
set -e

cd "$(dirname "$0")"
docker build -t state_serialization:latest .
docker run --rm state_serialization:latest -s
