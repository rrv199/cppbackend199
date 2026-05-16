FROM ubuntu:22.04 as builder

RUN apt update && apt install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY solution/ /app/

RUN rm -rf build && mkdir build && cd build && \
    cmake .. && \
    make -j4

FROM ubuntu:22.04

RUN apt update && apt install -y \
    libboost-serialization1.74-dev \
    libboost-filesystem1.74-dev \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/serialization_tests /app/game_server

WORKDIR /app

EXPOSE 8080

ENTRYPOINT ["./game_server"]
CMD ["-s"]
