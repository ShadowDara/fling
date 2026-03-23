// Header File for the Interpreter

#pragma once

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>
#include <memory>
#include <cassert>

#include "dcorelib/dcorelib.h"

#include "values.hpp"
#include "envirments.hpp"
#include "../frontend/ast.hpp"
#include "../util.hpp"
#include "eval/statements.hpp"
#include "eval/expressions.hpp"


namespace fling
{
	namespace runtime
	{
		// WHY DOES THIS FUNCTION EXISTS WTF???
		//
		// inline RuntimeVal evaluate(
    	// 	const ast::Program& program,
    	// 	runtime::envirment::Environment& env)
		// {
    	// 	return evaluate_program(program, env);
		// }

		// Function to evaluate source Code
		fling::runtime::RuntimeVal evaluate(
			const ast::Stmt& astNode,
			fling::runtime::envirment::Environment& env);
	} // namespace fling
} // namespace fling

#endif // INTERPRETER_HPP
