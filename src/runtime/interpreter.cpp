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
            const fling::ast::Program& program,
            runtime::envirment::Environment& env)
        {
            // Store the last evaluated value, null as Default
            runtime::RuntimeVal last = runtime::RuntimeVal();
            // loop through all statements in the program body
            for (const auto& stmt : program.body)
            {
                // STMT Deferenzieren to convert it to stmt&
                last = evaluate(*stmt, env);
            }

            // Return the last evaluated value
            return last;
        }

        // Function to Evaluate / Calculate the 2 Numbers
        runtime::RuntimeVal evaluate_numeric_binary_expr(
            runtime::RuntimeVal lhs,
            runtime::RuntimeVal rhs,
            std::string callculation_operator,
            runtime::envirment::Environment& env)
        { 
            float result = 0.0f;

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
                result = (rhs.number != 0) ? lhs.number / rhs.number : 0.0f;
            }
            // Module
            else if (callculation_operator == "%")
            {
                result = static_cast<float>(toInt(rhs.number) % toInt(lhs.number));
            }
            // Error
            else {
                // Unbekannter Operator, Fehlerbehandlung
                result = 0.0f;
            }

            return runtime::RuntimeVal(result);
        }

        // Function to evaluate a Binary Expression
        runtime::RuntimeVal evaluate_binary_expr(
            const ast::BinaryExpr& binop,
            runtime::envirment::Environment& env)
        {
            // Referrencing them!
            auto lhs = evaluate(*binop.left, env);
            auto rhs = evaluate(*binop.right, env);

            if (lhs.type == runtime::RuntimeVal::Type::Number
                && rhs.type == runtime::RuntimeVal::Type::Number)
            {
                return evaluate_numeric_binary_expr(
                    lhs, rhs, binop.callculation_operator, env);
            }

            // One or both are Null
            return runtime::RuntimeVal();
        }

        // Function to evaluate an Identifier
        runtime::RuntimeVal evaluate_identifier(
            const ast::Identifier& ident,
            runtime::envirment::Environment& env)
        { 
            return env.lookupVar(ident.symbol);
        }

		// Function to evaluate an Assignment Expression
        runtime::RuntimeVal evaluate_assignment_expr(
            const ast::AssignmentExpr& node,
            runtime::envirment::Environment& env)
        {
            if (node.assignme->kind != ast::NodeType::Identifier)
            {
				std::cerr <<
                    "Left-hand side of assignment must be an identifier."
                    << std::endl;
            }

            auto varName = static_cast<ast::Identifier*>(node.assignme.get())->symbol;

            // Use a Reference instead of a unique Pointer
            return env.assignVar(varName, evaluate(*node.value, env));
        }

		// Function to evaluate a Variable Declaration
        runtime::RuntimeVal evaluate_var_declaration(
            const ast::VarDeclaration& varDecl,
            runtime::envirment::Environment& env)
        { 
            // Use a Reference instead of a smart pointer
            auto value = varDecl.value ? evaluate(*varDecl.value, env) : runtime::RuntimeVal();
			return env.declareVar(varDecl.identifier, value, varDecl.constant);
        }

        // Function to evaluate Source Code
        runtime::RuntimeVal evaluate(
            const ast::Stmt& astNode,
            runtime::envirment::Environment& env)
        {
            switch (astNode.kind)
            {

            // Number Value
            case ast::NodeType::NumericLiteral:
            {
                auto& numNode = static_cast<const ast::NumericLiteral&>(astNode);
                return RuntimeVal(static_cast<float>(numNode.value));
            }

            // Identifier
            case ast::NodeType::Identifier:
            {
                auto& identNode = static_cast<const ast::Identifier&>(astNode);
                return env.lookupVar(identNode.symbol);
            }

			// Assignment Expression
            case ast::NodeType::AssignmentExpr:
            {
                auto& assignexpr = static_cast<const ast::AssignmentExpr&>(astNode);  // ⚡ Referenz!
                return evaluate_assignment_expr(assignexpr, env);
            }

            // Binary Expression
            case ast::NodeType::BinaryExpr:
            {
                auto& binNode = static_cast<const ast::BinaryExpr&>(astNode);
                auto lhs = evaluate(*binNode.left, env);  // dereferenzieren
                auto rhs = evaluate(*binNode.right, env);
                return evaluate_numeric_binary_expr(lhs, rhs, binNode.callculation_operator, env);
            }

            // Program Node
            case ast::NodeType::Program:
            {
                auto& prog = static_cast<const ast::Program&>(astNode); // Reference
                return runtime::evaluate_program(prog, env);
            }

            // Handle Statement Nodes
            case ast::NodeType::VarDeclaration:
            {
                auto& declNode = static_cast<const ast::VarDeclaration&>(astNode);
                auto value = declNode.value ? evaluate(*declNode.value, env) : RuntimeVal();
                return env.declareVar(declNode.identifier, value, declNode.constant);
            }

            // Error Fallback
            default:
            {
                cout << "Unknown AST Node Type: "
                    << static_cast<int>(astNode.kind)
                    << endl;
                dcorelib::Exit(1);
                return RuntimeVal();
            }

            }
        }
    } // namespace runtime
} // namespace fling
