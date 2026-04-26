// Header for the Values

#pragma once

#ifndef VALUES_HPP
#define VALUES_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include "../frontend/ast.hpp"


namespace fling::runtime::envirment {
    class Environment;
}


namespace fling::runtime
{
    //// Einfacher Typ f�r Null oder Number
    //struct RuntimeVal
    //{
    //    enum class Type {
    //        Null,
    //        Number,
    //        Boolean
    //    } type;

    //    float number;
    //    bool boolean;

    //    // Null Konstruktor
    //    RuntimeVal() : type(Type::Null), number(0) {};

    //    // float Number Konstruktor
    //    RuntimeVal(float n) : type(Type::Number), number(n) {}

    //    // Boolean Konstruktor
    //    //RuntimeVal()

    //    std::string toString() const
    //    {
    //        switch (type)
    //        {

    //        // Null
    //        case (Type::Null): {
    //            return "null";
    //        }

    //        // Number
    //        case (Type::Number): {
    //            return std::to_string(number);
    //        }

    //        // Boolean
    //        case (Type::Boolean): {
    //            //return std::to_string(boolean);
    //        }
    //        };

    //        /*float asNumber() const
    //        {
    //            if (type != Type::Number)
    //            {
    //                throw std::runtime_error("Value is not a number");
    //            }
    //            return number;
    //        }*/
    //    }
    //};

    struct RuntimeVal {
        enum class Type {
            Null,
            Number,
            Boolean,
            Object,
            Array,
            Native_FnValue,
            FnValue
        };

        Type type;

        // Number Type
        float number = 0;
        
        // Boolean Type
        bool bvalue = false;
        
        // Object Type
        std::unordered_map<std::string, std::unique_ptr<RuntimeVal>> properties;

        // Array Type
        std::vector<RuntimeVal> elements;
        
        // Native Function Type
        std::function <RuntimeVal(const std::vector<RuntimeVal>&, envirment::Environment&)> call;

        // for Function Type
        std::string name;
        std::vector<std::string> parameters;
        std::shared_ptr<envirment::Environment> declaration = nullptr;
        std::vector<std::unique_ptr<ast::Stmt>> body;

        // to make a Number Value
        static RuntimeVal Null()
        {
            return RuntimeVal{};
        }

        // to Make a Number Value
        static RuntimeVal Number(float n)
        {
            return RuntimeVal(static_cast<float>(n));
        }

        // to Make a Boolean Value
        static RuntimeVal Boolean(bool b)
        {
            return RuntimeVal(static_cast<bool>(b));
        }

        // Make an Object Value
        static RuntimeVal Object()
        {
            std::unordered_map<std::string, std::unique_ptr<RuntimeVal>> properties;
            auto val = RuntimeVal(std::move(properties));
            return val;
        }

        // Make an Array Value
        static RuntimeVal Array(std::vector<RuntimeVal> elements)
        {
            return RuntimeVal(std::move(elements));
        }

        // Make a Native Function
        static RuntimeVal NativeFN(
            std::function <RuntimeVal(const std::vector<RuntimeVal>&, envirment::Environment&)> call)
        {
            auto val = RuntimeVal(call);
            return val;
        }

        // Make a Function Value
        static RuntimeVal Function(
            std::string name,
            std::vector<std::string> params,
            std::shared_ptr<envirment::Environment> decl,
            std::vector<std::unique_ptr<ast::Stmt>> body
        ) {
            return RuntimeVal(
                std::move(name),
                std::move(params),
                std::move(decl),
                std::move(body)
            );
        }

        // Null Konstruktor
        RuntimeVal() : type(Type::Null) {}

        // float Kontruktor
        RuntimeVal(float n) : type(Type::Number), number(n) {}

        // Boolean Constuctor
        // This could mabybe lead to errors
        RuntimeVal(bool b) : type(Type::Boolean), bvalue(b) {}

        // Object Construktor
        RuntimeVal(std::unordered_map<std::string,
            std::unique_ptr<RuntimeVal>> p) : type(Type::Object), properties(std::move(p)) {};

        // Array Constructor
        RuntimeVal(std::vector<RuntimeVal> elems)
            : type(Type::Array),
              elements(std::move(elems)) {}

        // Native Function Construktor
        RuntimeVal(
            std::function <RuntimeVal(const std::vector<RuntimeVal>&, envirment::Environment&)> c)
            : type(Type::Native_FnValue), call(c) {};

        // Function Construktor
        RuntimeVal(
            std::string name,
            std::vector<std::string> params,
            std::shared_ptr<envirment::Environment> decl,
            std::vector<std::unique_ptr<ast::Stmt>> body
        )
            : type(Type::FnValue),
            name(std::move(name)),
            parameters(std::move(params)),
            declaration(std::move(decl)),
            body(std::move(body)) {
        }

        // Convert to String
        std::string toString() const
        {
            std::string return_msg = "{ type: ";

            switch (type)
            {
                case Type::Null:
                {
                    return_msg += "\"null\"";
                    return_msg += ", value: null";
                    break;
                }

                case Type::Number:
                {
                    return_msg += "\"number\"";
                    return_msg += ", value: " + std::to_string(number);
                    break;
                }

                case Type::Boolean:
                {
                    return_msg += "\"boolean\"";
                    return_msg += ", value: ";
                    return_msg += (bvalue ? "true" : "false");
                    break;
                }

                case Type::Object:
                {
                    return_msg += "\"object\"";
                    return_msg += ", properties: { ";

                    bool first = true;
                    for (const auto& [key, val] : properties)
                    {
                        if (!first) return_msg += ", ";
                        return_msg += key + ": " + val->toString();
                        first = false;
                    }

                    return_msg += " }";
                    break;
                }

                case Type::Array:
                {
                    return_msg += "\"array\"";
                    return_msg += ", values: [";

                    for (size_t i = 0; i < elements.size(); ++i)
                    {
                        if (i > 0) return_msg += ", ";
                        return_msg += elements[i].toString();
                    }

                    return_msg += "]";
                    break;
                }

                case Type::Native_FnValue:
                {
                    return_msg += "\"native-fn\"";
                    return_msg += ", value: <function>";
                    break;
                }

                default:
                {
                    return_msg += "\"unknown\"";
                    return_msg += ", value: ?";
                }
            }

            return_msg += " }";
            return return_msg;
        }

        // Copy Constructor
        RuntimeVal(const RuntimeVal& other)
            : type(other.type),
              number(other.number),
              bvalue(other.bvalue),
              call(other.call),
              name(other.name),
              parameters(other.parameters),
              declaration(other.declaration)
        {
            properties.reserve(other.properties.size());
            for (const auto& [key, value] : other.properties)
            {
                if (value)
                {
                    properties.emplace(key, std::make_unique<RuntimeVal>(*value));
                }
            }

            body.reserve(other.body.size());
            for (const auto& stmt : other.body)
            {
                if (stmt)
                {
                    body.push_back(stmt->clone());
                }
            }

            elements = other.elements;
        }

        RuntimeVal& operator=(const RuntimeVal& other)
        {
            if (this == &other)
            {
                return *this;
            }

            type = other.type;
            number = other.number;
            bvalue = other.bvalue;
            call = other.call;
            name = other.name;
            parameters = other.parameters;
            declaration = other.declaration;

            properties.clear();
            properties.reserve(other.properties.size());
            for (const auto& [key, value] : other.properties)
            {
                if (value)
                {
                    properties.emplace(key, std::make_unique<RuntimeVal>(*value));
                }
            }

            body.clear();
            body.reserve(other.body.size());
            for (const auto& stmt : other.body)
            {
                if (stmt)
                {
                    body.push_back(stmt->clone());
                }
            }

            elements = other.elements;

            return *this;
        }

        RuntimeVal(RuntimeVal&&) = default;
        RuntimeVal& operator=(RuntimeVal&&) = default;
    };
} // namespace fling::runtime

#endif // VALUES_HPP
