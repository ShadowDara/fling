build:
	echo Building Release
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

.PHONY: build
