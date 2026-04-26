#include "envirments.hpp"

using namespace fling;
using namespace fling::runtime;
using namespace fling::runtime::envirment;

// Function to setup the Standard Envirment for the Language
void envirment::setupStandardEnvironment(Environment& env)
{
    env.declareVar("true", RuntimeVal::Boolean(true), true);
    env.declareVar("false", RuntimeVal::Boolean(false), true);
    env.declareVar("null", RuntimeVal::Null(), true);

    // Define Print Function
    env.declareVar(
        "print",
        RuntimeVal::NativeFN([](const std::vector<RuntimeVal>& args, fling::runtime::envirment::Environment&) -> RuntimeVal {
            for (const auto& arg : args) {
                switch (arg.type) {
                    case RuntimeVal::Type::Number:
                        std::cout << arg.number << " ";
                        break;
                    case RuntimeVal::Type::Boolean:
                        std::cout << (arg.bvalue ? "true" : "false") << " ";
                        break;
                    case RuntimeVal::Type::Null:
                        std::cout << "null ";
                        break;
                    case RuntimeVal::Type::Object:
                        std::cout << "<object> ";
                        break;
                    case RuntimeVal::Type::Array:
                        std::cout << "Can not print <array>";
                        break;
                    default:
                        std::cout << "<unknown> ";
                        break;
                }
            }
            std::cout << std::endl;
            return RuntimeVal::Null();
        }),
        true
    );
}

// Function to declare a New Variable
RuntimeVal Environment::declareVar(
	std::string varName, RuntimeVal value, bool constant)
{
    // Check if variable already exists in THIS environment
    if (variables.find(varName) != variables.end())
    {
        std::string error_msg = "Can not redeclare Variable! Variable is already declared: " + varName;

        //throw std::runtime_error("Variable already declared: " + varName);

        std::cout << error_msg << std::endl;

        return RuntimeVal();
    }

    variables[varName] = std::move(value);

	// Add to the set of constants if this variable is
    // declared as constant
    if (constant)
    {
        constants.insert(varName);
	}

    return value;
}

// Function to assign a Variable
RuntimeVal Environment::assignVar(
    std::string varName, RuntimeVal value)
{
    auto env = this->resolve(varName);

	// Can not assign to a variable that is declared as constant
	if (env->constants.find(varName) != env->constants.end())
    {
        std::string error_msg = "Cannot assign to constant variable: " + varName;
        //throw std::runtime_error("Cannot assign to constant variable: " + varName);
        std::cout << error_msg << std::endl;
        return RuntimeVal();
    }

    env->variables[varName] = std::move(value);

    return value;
}

// Function to get the Content of a Variable
RuntimeVal Environment::lookupVar(std::string varName)
{
    auto env = this->resolve(varName);
    return env->variables[varName];
}

// Function to check if the Variable exists in the current scope
Environment* Environment::resolve(const std::string& varName)
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

bool Environment::hasVar(const std::string& varName) const
{
    if (this->variables.find(varName) != this->variables.end())
    {
        return true;
    }

    if (this->parent == nullptr)
    {
        return false;
    }

    return this->parent->hasVar(varName);
}
