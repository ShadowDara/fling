// Header for the Values

#pragma once

#ifndef VALUES_HPP
#define VALUES_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>


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
            Native_FnValue
        };

        Type type;
        float number = 0;
        bool bvalue = false;
        std::unordered_map<std::string, RuntimeVal> properties;
        std::function <RuntimeVal(std::vector<RuntimeVal>, envirment::Environment&)> call;

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
            std::unordered_map<std::string, RuntimeVal> properties;
            auto val = RuntimeVal(properties);
            return val;
        }

        // Make a Native Function
        static RuntimeVal NativeFN(
            std::function <RuntimeVal(std::vector<RuntimeVal>, envirment::Environment&)> call)
        {
            auto val = RuntimeVal(call);
            return val;
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
            RuntimeVal> p) : type(Type::Object), properties(p) {};

        // Native Function Construktor
        RuntimeVal(
            std::function <RuntimeVal(std::vector<RuntimeVal>, envirment::Environment&)> c)
            : type(Type::Native_FnValue), call(c) {};

        // Convert to String
        std::string toString() const
        {
            std::string return_msg = "{ type: ";

            switch (type)
            {
                case Type::Null: {
                    return_msg += "\"null\"";
                    return_msg += ", value: null";
                    break;
                }

                case Type::Number: {
                    return_msg += "\"number\"";
                    return_msg += ", value: " + std::to_string(number);
                    break;
                }

                case Type::Boolean: {
                    return_msg += "\"boolean\"";
                    return_msg += ", value: ";
                    return_msg += (bvalue ? "true" : "false");
                    break;
                }

                case Type::Object: {
                    return_msg += "\"object\"";
                    return_msg += ", properties: { ";

                    bool first = true;
                    for (const auto& [key, val] : properties)
                    {
                        if (!first) return_msg += ", ";
                        return_msg += key + ": " + val.toString();
                        first = false;
                    }

                    return_msg += " }";
                    break;
                }

                case Type::Native_FnValue: {
                    return_msg += "\"native-fn\"";
                    return_msg += ", value: <function>";
                    break;
                }
            }

            return_msg += " }";
            return return_msg;
        }
    };
} // namespace fling::runtime

#endif // VALUES_HPP
