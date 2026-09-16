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
#include <memory> // Für std::shared_ptr

namespace fling
{
    // Function to run a File
    void runFile(const std::string& filename);


    void runREPL();

    // Function to run Code
    void runCode(const std::string& code);

    // Function to create a new Envirment
    std::shared_ptr<fling::runtime::envirment::Environment> createEnvirment();

    // Function to run fling in this envirment
    void runCodeInEnvirment(const std::string& code,
        std::shared_ptr<fling::runtime::envirment::Environment> env);
}
