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
    make -j4

WORKDIR /app/build

ENTRYPOINT ["./serialization_tests"]
CMD ["-s"]
