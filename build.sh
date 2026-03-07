#/bin/bash

# Build with ZIG

zig c++ -O0 -std=c++20 main.cpp -o fling
