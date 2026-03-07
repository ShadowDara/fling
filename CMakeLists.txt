# CMakeList.txt: CMake-Projekt für "fling-lang". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_COLOR_MAKEFILE ON)

# Aktivieren Sie Hot Reload für MSVC-Compiler, sofern unterstützt.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("fling-lang")

# add my Corelib
add_subdirectory(thirdparty/dcorelib)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (
    fling-lang 
    "main.cpp"

    
    "src/frontend/ast.hpp"

    "src/frontend/lexer.cpp"
    "src/frontend/lexer.hpp"

    "src/frontend/parser.cpp"
    "src/frontend/parser.hpp"

    "src/runtime/values.hpp"
    

    "src/runtime/interpreter.hpp"
    "src/runtime/interpreter.cpp"
 "src/runtime/envirments.cpp" "src/util.cpp" "src/runtime/values.cpp" "src/runtime/eval/statements.cpp" "src/runtime/eval/expressions.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET fling-lang PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE dcorelib)

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
