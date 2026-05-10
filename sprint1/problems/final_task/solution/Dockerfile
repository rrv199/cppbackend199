# Сборочный контейнер
FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.*

# Копируем исходники map_json
COPY ./src /app/src
COPY CMakeLists.txt /app/
COPY conanfile.txt /app/

RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Запуск контейнер
FROM ubuntu:22.04 as run

# Создание пользователя для безопасности
RUN groupadd -r www && useradd -r -g www www
USER www

# Копирование бинарника и данных из сборочного контейнера
COPY --from=build /app/build/bin/game_server /app/
COPY ./data /app/data

# Точка входа
ENTRYPOINT ["/app/game_server", "/app/data/config.json"]
