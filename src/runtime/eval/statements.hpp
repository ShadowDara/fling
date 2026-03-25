#pragma once

#ifndef STATEMENTS_H
#define STATEMENTS_H

#include <memory>

#include "dcorelib/dcorelib.h"

#include "../interpreter.hpp"
#include "../values.hpp"
#include "../envirments.hpp"
#include "../../frontend/ast.hpp"
#include "../../util.hpp"


namespace fling::runtime::eval {
    // Function to evaluate a Program Node
	fling::runtime::RuntimeVal evaluate_program(
		const ast::Program& program,
        fling::runtime::envirment::Environment& env);

    // Function to evaluate a Variable Declaration
	fling::runtime::RuntimeVal evaluate_var_declaration(
		const ast::VarDeclaration& varDecl,
		runtime::envirment::Environment& env);

	// Function to evalua a Function Declaration
	fling::runtime::RuntimeVal evaluate_fn_declaration(
		const ast::FunctionDeclaration& fnDecl,
		runtime::envirment::Environment& env);
}

#endif // STATEMENTS_H
