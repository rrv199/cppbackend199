# Сборочный контейнер
FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.*

COPY . /app
WORKDIR /app

RUN mkdir build && cd build && \
    conan install .. --build=missing && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Запуск контейнер
FROM ubuntu:22.04 as run

RUN apt update && \
    apt install -y libstdc++6 && \
    rm -rf /var/lib/apt/lists/*

RUN groupadd -r www && useradd -r -g www www
USER www

COPY --from=build /app/build/bin/game_server /app/
COPY ./data /app/data
COPY ./static /app/static

WORKDIR /app

ENTRYPOINT ["/app/game_server", "/app/data/config.json", "/app/static"]
