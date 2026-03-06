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
#include "eval/statements.hpp"


namespace fling
{
	namespace runtime
	{
		// Function to evaluate a Program Node
		fling::runtime::RuntimeVal evaluate_program(
			const std::unique_ptr<ast::Program>& program,
			std::unique_ptr<fling::runtime::envirment::Environment>& env);

		// Function to evaluate a Numeric Binary Expression
		// with 2 Numbers
		fling::runtime::RuntimeVal evaluate_numeric_binary_expr(
			fling::runtime::RuntimeVal lhs,
			fling::runtime::RuntimeVal rhs,
			std::string callculation_operator,
			fling::runtime::envirment::Environment* env);

		// Function to evaluate a Binary Expression
		fling::runtime::RuntimeVal evaluate_binary_expr(const std::unique_ptr<ast::BinaryExpr>& binop,
			std::unique_ptr<fling::runtime::envirment::Environment>& env);

		// Function to evaluate an Identifier
		fling::runtime::RuntimeVal evaluate_identifier(const std::unique_ptr<ast::Identifier>& ident,
			std::unique_ptr<fling::runtime::envirment::Environment>& env);

		// Function to evaluate a Variable Declaration
		fling::runtime::RuntimeVal evaluate_var_declaration(
			const std::unique_ptr<ast::VarDeclaration>& varDecl,
			std::unique_ptr<runtime::envirment::Environment>& env);

		// Function to evaluate source Code
		fling::runtime::RuntimeVal evaluate(const std::unique_ptr<ast::Stmt>& astNode,
			std::unique_ptr<fling::runtime::envirment::Environment>& env);
	} // namespace fling
} // namespace fling

#endif // INTERPRETER_HPP
