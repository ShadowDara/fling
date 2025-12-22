// Header File for the Interpreter

#pragma once

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>

#include "dcorelib/dcorelib.h"

#include "values.hpp"
#include "envirments.hpp"
#include "../frontend/ast.hpp"
#include "../util.hpp"

namespace fling
{
	namespace runtime
	{
		// Function to evaluate a Program Node
		fling::runtime::RuntimeVal evaluate_program(ast::Program *program,
			fling::runtime::envirment::Environment* env);

		// Function to evaluate a Numeric Binary Expression
		// with 2 Numbers
		fling::runtime::RuntimeVal evaluate_numeric_binary_expr(
			fling::runtime::RuntimeVal lhs,
			fling::runtime::RuntimeVal rhs,
			std::string callculation_operator,
			fling::runtime::envirment::Environment* env);

		// Function to evaluate a Binary Expression
		fling::runtime::RuntimeVal evaluate_binary_expr(ast::BinaryExpr *binop,
			fling::runtime::envirment::Environment* env);

		// Function to evaluate an Identifier
		fling::runtime::RuntimeVal evaluate_identifier(ast::Identifier* ident,
			fling::runtime::envirment::Environment* env);

		// Function to evaluate source Code
		fling::runtime::RuntimeVal evaluate(ast::Stmt *astNode,
			fling::runtime::envirment::Environment* env);
	} // namespace fling
} // namespace fling

#endif // INTERPRETER_HPP
