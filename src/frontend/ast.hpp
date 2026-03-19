// ast.hpp

/**
 * Header file for the Abstract Syntax Tree for the Language
 */

#pragma once

#ifndef AST_HPP
#define AST_HPP

#include <string_view>
#include <vector>
#include <string>
#include <ostream>
#include <memory>


namespace fling
{
    namespace ast
    {
        /**
         * Function to prodruce an Indent String
         *
         * @param int for the indent counter
         * @return std::string for indents
         */
        static std::string indentStr(int indent)
        {
            return std::string(indent, ' ');
        }


        /**
         * enum class for the NodeTypes for the Abstract Syntax Tree
         */
        enum class NodeType
        {
            // STATEMENTS
            Program,
            VarDeclaration,

            // EXPRESSIONS
            AssignmentExpr,
            MemberExpr,
			CallExpr,

            // Literals
            Property,
            ObjectLiteral,
            NumericLiteral,
            Identifier,
            BinaryExpr,
            // CallExpr,
            // UnaryExpr,
            // FunctionDeclaration
        };


        /**
         * Statement a Basestructure for the abstract Syntax Tree
         *
         * @param Nodetype
         */
        struct Stmt {
            NodeType kind;

            explicit Stmt(NodeType kind) : kind(kind) {}
            virtual ~Stmt() = default;

            virtual std::string toString(int indent = 0) const {
                return "<Stmt>";
            }
        };


        // Program
        struct Program : Stmt
        {
            std::vector<std::unique_ptr<Stmt>> body;

            // toString function
            std::string toString(int indent = 0) const override
            {
                std::string out = indentStr(indent) + "Program:\n";

                for (const auto& stmt : body)
                {
                    out += stmt->toString(indent + 2);
                    // Add newline After each statement
                    out += "\n";
                }

                return out;
            }

            Program() : Stmt(NodeType::Program) {}        // Liste von Statements
            Program(Program&&) = default;
            Program& operator=(Program&&) = default;
            Program(const Program&) = delete;
            Program& operator=(const Program&) = delete;
        };


        // Ausdruck
        struct Expr : Stmt
        {
            explicit Expr(NodeType kind) : Stmt(kind) {}
        };


		// Assignment Expression
        //
        //
        struct AssignmentExpr : Expr
        {
            std::unique_ptr<ast::Expr> assignme;
            std::unique_ptr<ast::Expr> value;

			AssignmentExpr() : Expr(NodeType::AssignmentExpr) {}
            AssignmentExpr(AssignmentExpr&&) = default;
            AssignmentExpr& operator=(AssignmentExpr&&) = default;
            AssignmentExpr(const AssignmentExpr&) = delete;
            AssignmentExpr& operator=(const AssignmentExpr&) = delete;
        };


        // Variable Declaration
        struct VarDeclaration : Stmt
        {
            bool constant;
            std::string identifier;
            std::unique_ptr<ast::Expr> value;

            VarDeclaration() : Stmt(NodeType::VarDeclaration) {}
            VarDeclaration(VarDeclaration&&) = default;
            VarDeclaration& operator=(VarDeclaration&&) = default;
            VarDeclaration(const VarDeclaration&) = delete;
            VarDeclaration& operator=(const VarDeclaration&) = delete;
        };


        // Binary Expression
        struct BinaryExpr : Expr
        {
            std::unique_ptr<Expr> left;
            std::unique_ptr<Expr> right;
            std::string callculation_operator;

            // String-Konvertierung (optional)
            operator std::string() const
            {
                return "BinaryExpr";
            }

            // toString function
            std::string toString(int indent = 0) const override
            {
                std::string out = indentStr(indent) + "BinaryExpr:\n";
                out += indentStr(indent + 2) + "Left:\n";
                out += left->toString(indent + 4) + "\n";
                out += indentStr(indent + 2) + "Right:\n";
                out += right->toString(indent + 4) + "\n";
                out += indentStr(indent + 2) + "Binary Operator:\n";
                out += indentStr(indent + 4) + callculation_operator;
                return out;
            }

            BinaryExpr() : Expr(ast::NodeType::BinaryExpr) {}
            BinaryExpr(BinaryExpr&&) = default;
            BinaryExpr& operator=(BinaryExpr&&) = default;
            BinaryExpr(const BinaryExpr&) = delete;
            BinaryExpr& operator=(const BinaryExpr&) = delete;
        };


        // Call Expression
        struct CallExpr : Expr
        {
            std::vector<Expr> agrs;
            std::unique_ptr<Expr> caller;

            //// String-Konvertierung (optional)
            //operator std::string() const
            //{
            //    return "BinaryExpr";
            //}

            //// toString function
            //std::string toString(int indent = 0) const override
            //{
            //    std::string out = indentStr(indent) + "BinaryExpr:\n";
            //    out += indentStr(indent + 2) + "Left:\n";
            //    out += left->toString(indent + 4) + "\n";
            //    out += indentStr(indent + 2) + "Right:\n";
            //    out += right->toString(indent + 4) + "\n";
            //    out += indentStr(indent + 2) + "Binary Operator:\n";
            //    out += indentStr(indent + 4) + callculation_operator;
            //    return out;
            //}

            CallExpr(std::unique_ptr<Expr> c, std::vector<fling::ast::Expr> args
                ) : Expr(ast::NodeType::CallExpr), caller(std::move(c)), agrs(args) {}
            
            /*BinaryExpr(BinaryExpr&&) = default;
            BinaryExpr& operator=(BinaryExpr&&) = default;
            BinaryExpr(const BinaryExpr&) = delete;
            BinaryExpr& operator=(const BinaryExpr&) = delete;*/
        };


        // Member Expression
        struct MemberExpr : Expr
        {
            std::unique_ptr<Expr> object;
            std::unique_ptr<Expr> property;
			bool computed; // true for obj[prop], false for obj.prop

            //// String-Konvertierung (optional)
            //operator std::string() const
            //{
            //    return "BinaryExpr";
            //}

            //// toString function
            //std::string toString(int indent = 0) const override
            //{
            //    std::string out = indentStr(indent) + "BinaryExpr:\n";
            //    out += indentStr(indent + 2) + "Left:\n";
            //    out += left->toString(indent + 4) + "\n";
            //    out += indentStr(indent + 2) + "Right:\n";
            //    out += right->toString(indent + 4) + "\n";
            //    out += indentStr(indent + 2) + "Binary Operator:\n";
            //    out += indentStr(indent + 4) + callculation_operator;
            //    return out;
            //}

            MemberExpr(std::unique_ptr<Expr> o,
                std::unique_ptr<Expr> p,
                bool c) : Expr(ast::NodeType::MemberExpr),
                object(std::move(o)), property(std::move(p)),
                computed(c) {}
            /*BinaryExpr(BinaryExpr&&) = default;
            BinaryExpr& operator=(BinaryExpr&&) = default;
            BinaryExpr(const BinaryExpr&) = delete;
            BinaryExpr& operator=(const BinaryExpr&) = delete;*/
        };


        // Identifier
        struct Identifier : Expr
        {
            std::string symbol;

            // toString function
            std::string toString(int indent = 0) const override
            {
                return indentStr(indent) + "Identifier(" + symbol + ")";
            }

            // Standardkonstruktor
            Identifier() : Expr(NodeType::Identifier), symbol("") {}

            // Parameter-Konstruktor
            Identifier(std::string s)
                : Expr(NodeType::Identifier), symbol(std::move(s)) {
            }
        };


        // Numerisches Literal
        struct NumericLiteral : Expr
        {
            float value;

            // toString Function
            std::string toString(int indent = 0) const override
            {
                return indentStr(indent) + "NumericLiteral(" + std::to_string(value) + ")";
            }

            // Standardkonstruktor
            NumericLiteral() : Expr(NodeType::NumericLiteral), value(0) {}

            // Parameter-Konstruktor
            NumericLiteral(int v) : Expr(NodeType::NumericLiteral), value(v) {}
        };


		// Property f�r Objektliteral
        struct Property : Expr
        {
            std::string key;

            // Optional Value!
            std::unique_ptr<ast::Expr> value;
            
            // toString function
            std::string toString(int indent = 0) const override
            {
                std::string out = indentStr(indent) + "Property:\n";
                out += indentStr(indent + 2) + "Key: " + key + "\n";
                out += indentStr(indent + 2) + "Value:\n";
                out += value->toString(indent + 4);
                return out;
            }

            // Konstruktor
            Property() : Expr(NodeType::Property) {}
            
            Property(Property&&) = default;
            
            Property& operator=(Property&&) = default;
            
            Property(const Property&) = delete;
            
            Property& operator=(const Property&) = delete;
		};


		// Objektliteral
        struct ObjectLiteral : Expr
        {
            std::vector<std::unique_ptr<Property>> properties;
            
            std::string toString(int indent = 0) const override
            {
                std::string out = "{\n";

                for (size_t i = 0; i < properties.size(); ++i)
                {
                    const auto& prop = properties[i];

                    out += indentStr(indent + 2);
                    out += prop->key + ": ";

                    if (prop->value)
                        out += prop->value->toString(indent + 2);
                    else
                        out += "null";

                    if (i < properties.size() - 1)
                        out += ",";

                    out += "\n";
                }

                out += indentStr(indent) + "}";

                return out;
            }
            
			// Konstruktor
            ObjectLiteral() : Expr(NodeType::ObjectLiteral) {}
		};


        // for toString
        inline std::ostream &operator<<(std::ostream &os, const fling::ast::Stmt &stmt)
        {
            os << stmt.toString();
            return os;
        }


        // for toString
        inline std::ostream &operator<<(std::ostream &os, const fling::ast::Program &program)
        {
            os << program.toString();
            return os;
        }

    } // namespace ast
} // namespace fling

#endif // AST_HPP
