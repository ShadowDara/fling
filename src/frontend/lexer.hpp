// lexer.hpp

#pragma once

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string_view>
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>
#include <iostream>
#include <cctype>
#include <memory>

#include "dcorelib/dcorelib.h"


namespace fling
{
    namespace lexer
    {
        /**
         * @brief Enum for TokenTypes
         */
        enum TokenType
        {
            // Literal Types
            Number,
            Identifier,

            // Keywords
            Let,
            Const,

            Semicolon,
            Equals,
            OpenParen, // (
            CloseParen, // )

            Comma, // ,
            Colon, // :
            
            OpenSquaredBrace, // [
            CloseSquaredBrace, // ]

            // Calculation Operators
            BinaryOperator,

            //Illegal,
            Eof, // Signal end of File
        };

        /**
         * Token Struct Value
         *
         * @param string: value
         * @param TokenType type
         */
        struct Token
        {
            std::string value;
            TokenType type;
        };

        /**
         * Funktion to turn a Tokentype to a String
         *
         * @param TokenType as input argument
         * @return string from the TokenType
         */
        std::string tokenTypeToString(TokenType type);

        std::ostream &operator<<(std::ostream &os, const Token &token);

        /**
         * Check if a std::string is alphabetic
         *
         * INFO: The Function is overloaded
         *
         * @param std::string as the source which need to be checked
         * @return true if the string is alphabetic
         */
        bool isAlpha(const std::string &src);

        /**
         * Check if a single char is alphabetic
         *
         * INFO: The Function is overloaded
         *
         * @param char which should be checked
         * @return true if the char is alphabetic
         */
        bool isAlpha(char c);

        /**
         * Check if a std::string is an Integer
         *
         * @param std::string which should be checked
         * @return true if the input is an Integer
         */
        bool isInt(const std::string &str);

        /**
         * Chefck if a char is an valid Integer
         *
         * @param char which should be check
         * @return true if the char is an valid Integer
         */
        bool isInt(char c);

        /**
         * Check if a character is valid Whitespace
         *
         * @param char
         * @return true if the character is a whitespace
         */
        bool isSkippable(char c);

        /**
         * Function which generates and returns a new Token from both Input Values
         *
         * @param std::string, value for the Token
         * @param TokenType
         * @return A Token with both values
         */
        Token token(const std::string &value, TokenType type);

        /**
         * Tokenize Function which tokenises the source Code to an Array of Tokens
         *
         * @param std::string, the Input Code
         * @return an Vector of Tokens
         */
        std::vector<Token> tokenize(const std::string &input);

        extern std::unordered_map<std::string, TokenType> KEYWORDS;
    } // namespace lexer
} // namespace fling

#endif // LEXER_HPP
