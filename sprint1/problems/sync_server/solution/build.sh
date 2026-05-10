#!/bin/bash
mkdir -p build
cd build
conan install .. --build=missing
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j $(nproc)
