# Сборочный контейнер
FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      libboost-system-dev \
      libboost-thread-dev \
      libboost-filesystem-dev \
      cmake \
    && \
    rm -rf /var/lib/apt/lists/*

COPY ./src /app/src
COPY CMakeLists.txt /app/
COPY ./data /app/data

RUN mkdir /app/build && cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Запуск контейнер
FROM ubuntu:22.04 as run

RUN apt update && \
    apt install -y libboost-system1.74-dev libboost-thread1.74-dev && \
    rm -rf /var/lib/apt/lists/*

# Создание пользователя для безопасности
RUN groupadd -r www && useradd -r -g www www
USER www

# Копирование бинарника и данных из сборочного контейнера
COPY --from=build /app/build/hello /app/
COPY --from=build /app/data /app/data

# Точка входа
ENTRYPOINT ["/app/hello", "/app/data/config.json"]
