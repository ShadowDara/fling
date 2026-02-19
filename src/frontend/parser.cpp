// parser.cpp

#include "parser.hpp"

using namespace std;
using namespace fling;

namespace fling
{
    namespace parser
    {
        // match the namespace in the header

        // Implementation of Parser member functions

        // Function to check if end of file is reached
        bool Parser::not_eof()
        {
            return this->tokens[0].type != lexer::TokenType::Eof;
        }

        // Function to get the current token
        fling::lexer::Token Parser::at()
        {
            return this->tokens[0];
        }

        // Function to get the current token and remove it from the List
        fling::lexer::Token Parser::eat()
        {
            // save current
            fling::lexer::Token prev = this->tokens[0];

            // remove first element
            this->tokens.erase(this->tokens.begin());

            return prev;
        }

        // Function to expect a specific Token Type
        fling::lexer::Token Parser::expect(fling::lexer::TokenType type, string err)
        {
            fling::lexer::Token tk = this->eat();

            // Exit the program if the expected Token Type is not found
            if (tk.type != type)
            {
                cout << "Parser Error - Unexpected Token found: \n"
                     << err << " - "
                     << tk.value << " - Expecting: "
                     << fling::lexer::tokenTypeToString(type) << endl;
                cout << "Exiting Parser..." << endl;
                dcorelib::Exit(1);
            }

            return tk;
        }

        // Funktion to parse a Statement
        fling::ast::Stmt *Parser::parse_stmt()
        {
            switch (this->at().type)
            {
            // Let
            case (lexer::TokenType::Let):
            {
                return parse_var_declaration();
            }

            // Const
            case (lexer::TokenType::Const):
            {
                return parse_var_declaration();
            }

            // Default
            default:
            {
                /*if (this->at().type == lexer::TokenType::Eof)
                {
                    return nullptr;
                }*/

                // For now, we only parse expressions (identifiers / literals)
                // and wrap them as statements. Since Stmt is empty, you can
                // store Expr-derived nodes as Stmt* via Node* (or later extend Stmt)
                return reinterpret_cast<fling::ast::Stmt*>(parse_expr());
            }
            }
        }

        // Function to declare a new Variable
        // 
        // (CONST | LET) IDENTIFIER ;
        // 
        // (CONST | LET) IDENTIFIER = EXPR ;
        // 
        fling::ast::Stmt *Parser::parse_var_declaration()
        {
            bool isConstant =
                (this->eat().type == lexer::TokenType::Const);

            std::string identifier = this->expect(lexer::TokenType::Identifier,
                "Expected identifier name following let | const keywords").value;

            if (this->at().type == lexer::TokenType::Semicolon)
            {
                this->eat(); // Expect the Semicolon
                if (isConstant)
                {
                    std::cout <<
                        "Must assigne value to a Constant expression. No value provided."
                        << std::endl;
                    dcorelib::Exit(1);
                }

                ast::VarDeclaration* var = new ast::VarDeclaration();

                var->constant = isConstant;
                var->identifier = identifier;
                var->value = nullptr;

                return var;
            }

            this->expect(lexer::TokenType::Equals,
                "Expected equals Token following identifier in var declaration."
            );

            ast::VarDeclaration* declaration = new ast::VarDeclaration();

            declaration->value = this->parse_expr();
            declaration->constant = isConstant;
            declaration->identifier = identifier;
            
            this->expect(lexer::TokenType::Semicolon,
                "Variable declaration statement must end with semicolon.");

            return declaration;
        }

        // Funktion to parse an expression
        fling::ast::Expr *Parser::parse_expr()
        {
            return parse_additive_expr();
        }

        // Function to parse an additive Expression
        fling::ast::Expr *Parser::parse_additive_expr()
        {
            fling::ast::Expr *left = parse_multiplicitave_expr();

            // For Addition and Subtraction
            while (this->at().value == "+" || this->at().value == "-")
            {
                std::string callculation_operator = this->eat().value;
                fling::ast::Expr *right = this->parse_primary_expr();

                auto leftnew = new fling::ast::BinaryExpr();
                leftnew->left = left;
                leftnew->right = right;
                leftnew->callculation_operator = callculation_operator;

                left = leftnew;
            }

            return left;
        }

        // Function to parse an multiplicitave Expression
        fling::ast::Expr *Parser::parse_multiplicitave_expr()
        {
            fling::ast::Expr *left = parse_primary_expr();

            // For Division, Multiplication and Modulo
            while (
                this->at().value == "/" ||
                this->at().value == "*" ||
                this->at().value == "%")
            {
                std::string callculation_operator = this->eat().value;
                fling::ast::Expr *right = this->parse_multiplicitave_expr();

                auto leftnew = new fling::ast::BinaryExpr();
                leftnew->left = left;
                leftnew->right = right;
                leftnew->callculation_operator = callculation_operator;

                left = leftnew;
            }

            return left;
        }

        // Function to parse a float Value
        int Parser::parse_float(const std::string &value)
        {
            if (fling::lexer::isInt(value))
            {
                return std::stoi(value);
            }
            // Bei Fehler kannst du z.B. 0 oder einen speziellen Wert zurückgeben
            return 0;
        }

        // Funktion to parse a primary Expression
        fling::ast::Expr *Parser::parse_primary_expr()
        {
            auto tk = this->at();

            switch (tk.type)
            {

            // Identifier Type
            case fling::lexer::TokenType::Identifier:
            {
                // eat() returns the actual token and removes it
                auto token = this->eat();

                auto *id = new fling::ast::Identifier();
                id->symbol = token.value; // token.value is the identifier text

                return id; // implicit upcast to Expr*
            }

            // Number Type
            case fling::lexer::TokenType::Number:
            {
                auto *id = new fling::ast::NumericLiteral();
                id->value = parse_float(this->eat().value);
                // token.value is the Nummeric Literal value

                return id; // implicit upcast to Expr*
            }

            // Opening Parenthesis Type
            case fling::lexer::TokenType::OpenParen:
            {
                this->eat(); // Eat the opening Token
                auto *expr = this->parse_expr();
                this->expect(
                    fling::lexer::TokenType::CloseParen,
                    "Unexpected Token found inside parenthesised expression. Expected closing parenthesis"); // Eat the closing Parenthesis
                return expr;                                                                                 // Return the inner Expression
            }

            // Default Type for Unexpected Tokens
            default:
            {
                cout << "Unexpected Token found during Parsing: "
                     << this->at() << endl;
                dcorelib::Exit(1);
            }
            }
        }

        //
        //
        // Orders of Prescidence
        // Assignment Expr
        // Member Expr
        // Function Call
        // Logical Expr
        // Comparssion Expr
        // Aditive Expr
        // Multiplicitave Expr
        // Unary Expr
        // Primary Expr
        //
        //

        // Function to produec AST
        fling::ast::Program Parser::produceAST(const std::string &sourceCode)
        {
            this->tokens = fling::lexer::tokenize(sourceCode);

            fling::ast::Program program;

            while (this->not_eof())
            {
                auto *stmt = parse_stmt();
                if (stmt)
                {
                    program.body.push_back(stmt);
                }
            }

            return program;
        }
    } // namespace parser
} // namespace fling
