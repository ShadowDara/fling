// interpreter.cpp

#include "interpreter.hpp"

using namespace std;
using namespace fling;
using namespace fling::util;

namespace fling
{
    namespace runtime
    {
        // Function to evaluate a Program
        fling::runtime::RuntimeVal fling::runtime::evaluate_program(
            fling::ast::Program* program,
            runtime::envirment::Environment* env)
        {
            // Store the last evaluated value, null as Default
            runtime::RuntimeVal last = runtime::RuntimeVal();
            // loop through all statements in the program body
            for (auto* stmt : program->body)
            {
                last = evaluate(stmt, env);
            }

            // Return the last evaluated value
            return last;
        }

        // Function to Evaluate / Calculate the 2 Numbers
        runtime::RuntimeVal evaluate_numeric_binary_expr(
            runtime::RuntimeVal lhs,
            runtime::RuntimeVal rhs,
            std::string callculation_operator,
            runtime::envirment::Environment* env)
        { 
            float result = 0;

            // Additon
            if (callculation_operator == "+")
            {
                result = lhs.number + rhs.number;
            }
            // Subtraction
            else if (callculation_operator == "-")
            {
                result = lhs.number - rhs.number;
            }
            // Multiplikation
            else if (callculation_operator == "*")
            {
                result = lhs.number * rhs.number;
            }
            // Division
            else if (callculation_operator == "/")
            {
                // Division durch 0 vermeiden
                result = (rhs.number != 0) ? lhs.number / rhs.number : 0;
            }
            // Module
            else if (callculation_operator == "%")
            {
                result = toInt(rhs.number) % toInt(lhs.number);
            }
            // Error
            else {
                // Unbekannter Operator, Fehlerbehandlung
                result = 0;
            }

            return runtime::RuntimeVal(result);
        }

        // Function to evaluate a Binary Expression
        runtime::RuntimeVal evaluate_binary_expr(ast::BinaryExpr *binop,
            runtime::envirment::Environment* env)
        {
            auto lhs = evaluate(binop->left, env);
            auto rhs = evaluate(binop->right, env);

            if (lhs.type == runtime::RuntimeVal::Type::Number
                && rhs.type == runtime::RuntimeVal::Type::Number)
            {
                return evaluate_numeric_binary_expr(
                    lhs, rhs, binop->callculation_operator, env);
            }

            // One or both are Null
            return runtime::RuntimeVal();
        }

        // Function to evaluate an Identifier
        runtime::RuntimeVal evaluate_identifier(ast::Identifier* ident,
            runtime::envirment::Environment* env)
        { 
            return env->lookupVar(ident->symbol);
        }

		// Function to evaluate a Variable Declaration
        runtime::RuntimeVal evaluate_var_declaration(ast::VarDeclaration* varDecl,
            runtime::envirment::Environment* env)
        { 
			return RuntimeVal();
        }

        // Function to evaluate Source Code
        runtime::RuntimeVal evaluate(ast::Stmt *astNode,
            runtime::envirment::Environment* env)
        {
            switch (astNode->kind)
            {

            // Number Value
            case ast::NodeType::NumericLiteral:
            {
                auto numNode = static_cast<ast::NumericLiteral *>(astNode);
                return runtime::RuntimeVal(
                    static_cast<float>(numNode->value));
            }

            // Identifier
            case ast::NodeType::Identifier:
            {
                return evaluate_identifier(
                    static_cast<ast::Identifier *>(astNode), env);
            }

            // Binary Expression
            case ast::NodeType::BinaryExpr:
            {
                return evaluate_binary_expr(
                    static_cast<ast::BinaryExpr *>(astNode), env);
            }

            // Program Node
            case ast::NodeType::Program:
            {
				return runtime::evaluate_program(
                    static_cast<ast::Program*>(astNode), env);
            }

            // Handle Statement Nodes
            case::ast::NodeType::VarDeclaration:
            {
                return evaluate_var_declaration(
                    static_cast<ast::VarDeclaration*>(astNode), env);
            }

            // Error Fallback
            default:
            {
                cout << "Unknown AST Node Type: "
                    << static_cast<int>(astNode->kind)
                    << endl;
                dcorelib::Exit(1);
                return RuntimeVal();
            }

            }
        }
    } // namespace runtime
} // namespace fling
