# Makefile for Fling

build:
	echo Building Release
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run:
	$(MAKE) build
	./build/fling-lang

.PHONY: build
