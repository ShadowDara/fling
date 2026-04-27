#pragma once
#include <string>

#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "frontend/ast.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/envirments.hpp"

#include "rust/cxx.h"

void run_file(rust::Str filename);
