#include "envirments.hpp"

using namespace fling;
using namespace fling::runtime;
using namespace fling::runtime::envirment;

// Function to declare a New Variable
RuntimeVal Environment::declareVar(
	std::string varName, RuntimeVal value)
{
    // Check if variable already exists in THIS environment
    if (variables.find(varName) != variables.end())
    {
        std::string error_msg = "Variable already declared: " + varName;

        //throw std::runtime_error("Variable already declared: " + varName);

        std::cout << error_msg << std::endl;

        dcorelib::Exit(1);
    }

    variables[varName] = value;
    return value;
}

// Function to assign a Variable
RuntimeVal Environment::assignVar(
    std::string varName, RuntimeVal value)
{
    auto env = this->resolve(varName);
    env->variables[varName] = value;

    return value;
}

// Function to get the Content of a Variable
RuntimeVal Environment::lookupVar(std::string varName)
{
    auto env = this->resolve(varName);
    return env->variables[varName];
}

// Function to check if the Variable exists in the current scope
Environment* Environment::resolve(std::string varName)
{
    if (this->variables.find(varName) != this->variables.end())
    {
        return this;
    }

    if (this->parent == nullptr)
    {
        std::string error_msg = "Cannot resolve '" + varName + "' as it does not exist.";
        
        //throw std::runtime_error(error_msg);
        
        std::cout << error_msg << std::endl;

        //return nullptr;
        
        dcorelib::Exit(1);
    }

    return this->parent->resolve(varName);
}
