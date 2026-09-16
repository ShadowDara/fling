#include "fling.hpp"

// fling c++


using namespace fling;
using namespace fling::ast;
using namespace fling::lexer;
using namespace fling::parser;
using namespace fling::runtime;
using namespace fling::runtime::envirment;

using namespace std;

// Function to run a File
void fling::runFile(const std::string& filename)
{
    std::ifstream file{ filename };
    if (!file)
    {
        std::cerr << "Could not open file" << "\n";
        return;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()
    );

    Parser parser;
    auto env = std::make_shared<Environment>(nullptr);
    // envirment::setupStandardEnvironment(*env);

    Program program = parser.produceAST(content);
    //std::cout << "Print Program: " << program.toString() << "\n";

    auto result = evaluate(program, env);
    //std::cout << result.toString() << "\n";
}


void fling::runREPL()
{
    // Variable for the Source Code
    std::string source;

    // Parser for the source
    Parser parser;

    // Define the Envirment for the Language
    auto env = std::make_shared<Environment>(nullptr);
    // envirment::setupStandardEnvironment(*env);

    while (true)
    {
        std::getline(std::cin, source);

        try
        {
            // Produce AST from source Code
            Program program = parser.produceAST(source);

            RuntimeVal result = evaluate(program, env);
            cout << result.toString() << endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }
    }
}


// Function to run Code
void fling::runCode(const std::string& code)
{
    Parser parser;
    auto env = std::make_shared<Environment>(nullptr);
    // envirment::setupStandardEnvironment(*env);

    Program program = parser.produceAST(code);
    //std::cout << "Print Program: " << program.toString() << "\n";

    auto result = evaluate(program, env);
    //std::cout << result.toString() << "\n";
}

std::shared_ptr<Environment> fling::createEnvirment()
{
    auto env = std::make_shared<Environment>(nullptr);
    
    return std::move(env);
}

void fling::runCodeInEnvirment(const std::string& code, std::shared_ptr<Environment> env)
{
    Parser parser;
    Program program = parser.produceAST(code);
    //std::cout << "Print Program: " << program.toString() << "\n";
    auto result = evaluate(program, env);
    //std::cout << result.toString() << "\n";
}
