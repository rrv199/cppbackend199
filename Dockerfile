FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY sprint4/problems/state_serialization/solution/ /app/

RUN rm -rf build && mkdir build && cd build && \
    cmake .. && \
    make -j4 && \
    cp serialization_tests /app/game_server

WORKDIR /app

ENTRYPOINT ["./game_server"]
CMD ["-s"]
