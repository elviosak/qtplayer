#!/bin/bash
mkdir -p build
rm -rf build/*
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make -j$(nproc) && sudo make install
