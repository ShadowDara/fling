#include "statements.hpp"

using namespace fling;
using namespace fling::runtime;


// Function to evaluate a Program
fling::runtime::RuntimeVal fling::runtime::eval::evaluate_program(
    const fling::ast::Program &program,
    runtime::envirment::Environment &env)
{
    // Store the last evaluated value, null as Default
    runtime::RuntimeVal last = runtime::RuntimeVal::Null();
    // loop through all statements in the program body
    for (const auto &stmt : program.body)
    {
        // STMT Deferenzieren to convert it to stmt&
        last = evaluate(*stmt, env);
    }

    // Return the last evaluated value
    return last;
}


// Function to evaluate a Variable Declaration
runtime::RuntimeVal fling::runtime::eval::evaluate_var_declaration(
    const ast::VarDeclaration &varDecl,
    runtime::envirment::Environment &env)
{
    // Use a Reference instead of a smart pointer
    auto value = varDecl.value ? evaluate(*varDecl.value, env) : runtime::RuntimeVal();
    return env.declareVar(varDecl.identifier, value, varDecl.constant);
}
