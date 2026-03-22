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


namespace fling
{
	namespace runtime
	{
		// Function to evaluate a Program Node
		fling::runtime::RuntimeVal evaluate_program(
			const ast::Program& program,
            fling::runtime::envirment::Environment& env);
		
		// WHY DOES THIS FUNCTION EXISTS WTF???
		//
		// inline RuntimeVal evaluate(
    	// 	const ast::Program& program,
    	// 	runtime::envirment::Environment& env)
		// {
    	// 	return evaluate_program(program, env);
		// }
		
		fling::runtime::RuntimeVal evaluate_numeric_binary_expr(
			fling::runtime::RuntimeVal lhs,
			fling::runtime::RuntimeVal rhs,
			std::string callculation_operator,
			fling::runtime::envirment::Environment& env
		);

		// Function to evaluate a Binary Expression
		fling::runtime::RuntimeVal evaluate_binary_expr(
			const ast::BinaryExpr& binop,
			fling::runtime::envirment::Environment& env);

		// Function to evaluate an Identifier
		fling::runtime::RuntimeVal evaluate_identifier(
			const ast::Identifier& ident,
			fling::runtime::envirment::Environment& env);

		// Function to evaluate an Assignment Expression
		fling::runtime::RuntimeVal evaluate_assignment_expr(
			const ast::AssignmentExpr& node,
			fling::runtime::envirment::Environment& env);

		// Function to evaluate an Object Literal
		fling::runtime::RuntimeVal evaluate_object_expr(
			const ast::ObjectLiteral& node,
			fling::runtime::envirment::Environment& env);
		
		// Function to evaluate a Call Expression
		fling::runtime::RuntimeVal evaluate_call_expr(
            const ast::CallExpr& expr,
            fling::runtime::envirment::Environment& env);

		// Function to evaluate a Variable Declaration
		fling::runtime::RuntimeVal evaluate_var_declaration(
			const ast::VarDeclaration& varDecl,
			runtime::envirment::Environment& env);

		// Function to evaluate source Code
		fling::runtime::RuntimeVal evaluate(
			const ast::Stmt& astNode,
			fling::runtime::envirment::Environment& env);
	} // namespace fling
} // namespace fling

#endif // INTERPRETER_HPP
