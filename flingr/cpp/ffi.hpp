#pragma once
#include <string>

#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "frontend/ast.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/envirments.hpp"

void run_file(const std::string& filename);
