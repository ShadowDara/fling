#include "ffi.hpp"

// deine echten Includes
#include <fstream>
#include <iostream>

using namespace fling;
using namespace fling::ast;
using namespace fling::lexer;
using namespace fling::parser;
using namespace fling::runtime;
using namespace fling::runtime::envirment;


// interne Funktion (dein Originalcode)
void runFile(const std::string& filename)
{
    std::ifstream file{ filename };
    if (!file)
    {
        std::cerr << "Could not open file\n";
        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    Parser parser;
    auto env = std::make_shared<Environment>(nullptr);

    Program program = parser.produceAST(content);
    auto result = evaluate(program, *env);
}

// RUST WRAPPER FUNCTION
void run_file(rust::Str filename)
{
    std::string file_str(filename);

    runFile(file_str);
}
