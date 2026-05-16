#!/bin/bash
set -e

echo "=== Building Docker image ==="
docker build -t state_serialization:latest . 2>&1 | tee build.log

echo "=== Checking image ==="
docker images | grep state_serialization

echo "=== Running container in detached mode ==="
CONTAINER_ID=$(docker run -d -p 8080:8080 --name state_serialization_test state_serialization:latest)
echo "Container ID: $CONTAINER_ID"

echo "=== Waiting for container to start ==="
sleep 3

echo "=== Checking container status ==="
docker ps -a | grep state_serialization

echo "=== Container logs ==="
docker logs $CONTAINER_ID || echo "No logs yet"

echo "=== Running tests ==="
# Запускаем тесты с сохранением контейнера
docker run --rm \
  -v /var/run/docker.sock:/var/run/docker.sock \
  --network host \
  state_serialization:latest -s

echo "=== Tests completed ==="

# Останавливаем и удаляем контейнер
docker stop $CONTAINER_ID || true
docker rm $CONTAINER_ID || true

echo "=== Cleanup done ==="
