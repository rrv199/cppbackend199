FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY solution/ /app/

RUN rm -rf build && mkdir build && cd build && \
    cmake .. && \
    make -j4 && \
    cp serialization_tests /app/game_server && \
    echo "=== Build completed ===" && \
    ls -la /app/game_server

WORKDIR /app

# Даем права на выполнение
RUN chmod +x /app/game_server

# Проверяем, что бинарник работает
RUN /app/game_server -l

ENTRYPOINT ["./game_server"]
CMD ["-s"]
