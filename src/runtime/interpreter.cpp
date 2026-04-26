// interpreter.cpp

#include "interpreter.hpp"

using namespace std;
using namespace fling;
using namespace fling::util;
using namespace fling::ast;
using namespace fling::runtime;
//using namespace fling::runtime::eval;


namespace fling
{
    namespace runtime
    {
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

			// Object Literal
            case ast::NodeType::ObjectLiteral:
            {
                auto& objNode = static_cast<const ast::ObjectLiteral&>(astNode);
                return eval::evaluate_object_expr(objNode, env);
			}

            // Member Expression
            case ast::NodeType::MemberExpr:
            {
                auto& memExpr = static_cast<const ast::MemberExpr&>(astNode);
                auto objVal = evaluate(*memExpr.object, env);

                if (objVal.type != RuntimeVal::Type::Object)
                {
                    return RuntimeVal::Null();
                }

                std::string propertyKey;
                if (memExpr.computed)
                {
                    if (!memExpr.property)
                    {
                        return RuntimeVal::Null();
                    }

                    auto propVal = evaluate(*memExpr.property, env);
                    propertyKey = propVal.toString();
                }
                else
                {
                    auto propIdent = static_cast<ast::Identifier*>(memExpr.property.get());
                    if (!propIdent)
                    {
                        return RuntimeVal::Null();
                    }
                    propertyKey = propIdent->symbol;
                }

                auto it = objVal.properties.find(propertyKey);
                if (it == objVal.properties.end() || !it->second)
                {
                    return RuntimeVal::Null();
                }

                return *it->second;
            }

            // Call Expression
            case ast::NodeType::CallExpr:
            {
                auto& objNode = static_cast<const ast::CallExpr&>(astNode);
                return eval::evaluate_call_expr(objNode, env);
			}

			// Assignment Expression
            case ast::NodeType::AssignmentExpr:
            {
                auto& assignexpr = static_cast<const ast::AssignmentExpr&>(astNode);  // ⚡ Referenz!
                return eval::evaluate_assignment_expr(assignexpr, env);
            }

            // Binary Expression
            case ast::NodeType::BinaryExpr:
            {
                auto& binNode = static_cast<const ast::BinaryExpr&>(astNode);
                return eval::evaluate_binary_expr(binNode, env);
            }

            // Program Node
            case ast::NodeType::Program:
            {
                auto& prog = static_cast<const ast::Program&>(astNode); // Reference
                return eval::evaluate_program(prog, env);
            }

            // Handle Statement Nodes
            case ast::NodeType::VarDeclaration:
            {
                auto& declNode = static_cast<const ast::VarDeclaration&>(astNode);
                //auto value = declNode.value ? evaluate(*declNode.value, env) : RuntimeVal();
                //return env.declareVar(declNode.identifier, value, declNode.constant);
                return eval::evaluate_var_declaration(declNode, env);
            }

            // Handle Function Declaration
            case ast::NodeType::FunctionDeckaration:
            {
                auto& declNode = static_cast<const ast::FunctionDeclaration&>(astNode);
                return eval::evaluate_fn_declaration(declNode, env);
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
