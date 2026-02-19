// parser.hpp

#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

#include "lexer.hpp"
#include "ast.hpp"

#include "dcorelib/dcorelib.h"

namespace fling
{
    namespace parser
    {

        class Parser
        {
        private:
            /**
             * Vector Array for the List of Tokens
             */
            std::vector<fling::lexer::Token> tokens;

            /**
             * Function to check if the type of the current Token is equal to Eof
             *
             * @return Boolean for current tokentype and Eof Type
             */
            bool not_eof();

            /**
             * Parser function to get the current Token
             *
             * @return current Token
             */
            fling::lexer::Token at();

            /**
             * Parser function to get the current Token and remove it
             *
             * @return current Token
             */
            fling::lexer::Token eat();

            /**
             * Function to expect a Token of a specific Type
             *
             * @param TokenType which should be expected
             * @param string for any additional Error Message
             * @return Returns the expected Token
             */
            fling::lexer::Token expect(fling::lexer::TokenType type, std::string any);

            /**
             * Function to parse a statement
             *
             * @return Returns a statement from the source Code
             */
            fling::ast::Stmt *parse_stmt();

            /**
             * Function to parse a Variable Deklaration
             */
            fling::ast::Stmt* parse_var_declaration();

            /**
             * Function to parse an expression
             *
             * @return Returns a Expression from the current Token
             */
            fling::ast::Expr *parse_expr();

            /**
             * Function to parse an additive Expression
             *
             * @return an Additive Expression from the current Token
             */
            fling::ast::Expr *parse_additive_expr();

            /**
             * Function to parse a multiplicative Expression
             *
             * @return Returns a Multiplicative Expression from the current Token
             */
            fling::ast::Expr *parse_multiplicitave_expr();

            /**
             * Function to parse a float Value and convert it to a String
             *
             * @param std::string as a Float Value or Basic Number
             * @return Returns an Integer from the input string
             */
            int parse_float(const std::string &value);

            /**
             * Function to parse a primary expression from the current Token
             *
             * @return Returns a primary Expression
             */
            fling::ast::Expr *parse_primary_expr();

        public:
            /**
             * Generate an AST Program from source code
             *
             * @param std::string source Code
             * @return Program form the source Code
             */
            fling::ast::Program produceAST(const std::string &sourceCode);
        }; // Class Parser

    } // namespace parser
} // namespace fling

#endif // PARSER_HPP
