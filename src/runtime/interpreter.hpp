// Header File for the Interpreter

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>

#include "dcorelib/dcorelib.h"
#include "values.hpp"
#include "../frontend/ast.hpp"

namespace fling
{
	namespace runtime
	{
		// Function to evaluate a Program Node
		fling::runtime::RuntimeVal *evaluate_program(fling::ast::Program *program);

		// Function to evaluate a Numeric Binary Expression
		// with 2 Numbers
		fling::runtime::NumberVal* evaluate_numeric_binary_expr(
			fling::runtime::NumberVal* lhs, fling::runtime::NumberVal* rhs,
			std::string callculation_operator);

		// Function to evaluate a Binary Expression
		fling::runtime::RuntimeVal *evaluate_binary_expr(ast::BinaryExpr *binop);

		// Function to evaluate source Code
		fling::runtime::RuntimeVal *evaluate(fling::ast::Stmt *astNode);
	} // namespace fling
} // namespace fling

#endif // INTERPRETER_HPP
