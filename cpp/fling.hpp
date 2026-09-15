#pragma once

// Libary Header for fling
// fling c++

#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "frontend/ast.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/envirments.hpp"

#include <iostream>
#include <string>
#include <cassert> // Für assert()
#include <fstream>

namespace fling
{
    // Function to run a File
    void runFile(const std::string& filename);


    void runREPL();


    // Function to run Code
    void runCode(const std::string& code);
}
