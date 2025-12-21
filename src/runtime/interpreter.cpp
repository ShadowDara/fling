// interpreter.cpp

#include "interpreter.hpp"

using namespace std;

namespace fling
{
    namespace runtime
    {
        // Function to evaluate a Program
        runtime::RuntimeVal *evaluate_program(ast::Program *program)
        {
			// Store the last evaluated value, null as Default
			runtime::RuntimeVal* lastEvaluated = new runtime::NullVal();
            
			// loop through all statements in the program body
            for (auto* statement : program->body)
            { 
                lastEvaluated = evaluate(statement);
            }

			// Return the last evaluated value
            return lastEvaluated;
        }

        // Function to Evaluate / Calculate the 2 Numbers
        fling::runtime::NumberVal* evaluate_numeric_binary_expr(
            fling::runtime::NumberVal* lhs, fling::runtime::NumberVal *rhs,
            std::string callculation_operator)
        { 
            int result = 0;

            // Additon
            if (callculation_operator == "+")
            {
                result = lhs->value + rhs->value;
            }
            // Subtraction
            else if (callculation_operator == "-")
            {
                result = lhs->value - rhs->value;
            }
            // Multiplikation
            else if (callculation_operator == "*")
            {
                result = lhs->value * rhs->value;
            }
            // Division
            else if (callculation_operator == "/")
            {
                // Division durch 0 vermeiden
                result = (rhs->value != 0) ? lhs->value / rhs->value : 0;
            }
            // Module
            else if (callculation_operator == "%")
            {
                result = rhs->value % lhs->value;
            }
            // Error
            else {
                // Unbekannter Operator, Fehlerbehandlung
                result = 0;
            }

            return new NumberVal(result);
        }

        // Function to evaluate a Binary Expression
        runtime::RuntimeVal *evaluate_binary_expr(ast::BinaryExpr *binop)
        {
            auto lhs = evaluate(binop->left);
            auto rhs = evaluate(binop->right);

            if (lhs->type == runtime::ValueType::Number
                && rhs->type == runtime::ValueType::Number)
            {
                // Cast them from RuntimeVal to NumberVal
                auto clhs = static_cast<runtime::NumberVal*>(lhs);
                auto crhs = static_cast<runtime::NumberVal*>(rhs);

                return evaluate_numeric_binary_expr(
                    clhs, crhs, binop->callculation_operator);
            }

            // One or both are Null
            return new runtime::NullVal();
        }

        // Function to evaluate Source Code
        runtime::RuntimeVal *evaluate(ast::Stmt *astNode)
        {
            switch (astNode->kind)
            {

            // Number Value
            case ast::NodeType::NumericLiteral:
            {
                auto numNode = static_cast<ast::NumericLiteral *>(astNode);
                return new runtime::NumberVal(numNode->value); // korrekt: Pointer
            }

            // Null Value
            case ast::NodeType::NullLiteral:
            {
                return new runtime::NullVal(); // korrekt: Pointer
            }

            // Binary Expression
            case ast::NodeType::BinaryExpr:
            {
                return evaluate_binary_expr(static_cast<ast::BinaryExpr *>(astNode));
            }

            // Program Node
            case ast::NodeType::Program:
            {
				return evaluate_program(static_cast<ast::Program*>(astNode));
            }

            // Error Fallback
            default:
            {
                cout << "Unknown AST Node Type: "
                    << static_cast<int>(astNode->kind)
                    << endl;
                dcorelib::Exit(1);
                return new RuntimeVal();
            }
            }
        }
    }
}
