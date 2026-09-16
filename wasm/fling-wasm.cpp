// fling-wasm.cpp
// Alternative to wasm/build.mjs: a standalone CMake target
// that builds the Fling Wasm module with `--bind`.
//
// Add to CMakeLists.txt (with the WASM toolchain from Emscripten):
//
//   add_executable(fling-wasm cpp/wasm/em.cpp ${FLING_SOURCES})
//   target_include_directories(fling-wasm PRIVATE cpp)
//   set_target_properties(fling-wasm PROPERTIES
//       CXX_STANDARD 20
//       CXX_STANDARD_REQUIRED ON
//       LINK_FLAGS "--bind -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createFling -sENVIRONMENT=web,node -sALLOW_MEMORY_GROWTH=1 -sFILESYSTEM=0")
//
// Then configure with the Emscripten CMake toolchain:
//   emcmake cmake -B build-wasm
//   cmake --build build-wasm

#include "em.cpp" // single translation unit (headers already included)