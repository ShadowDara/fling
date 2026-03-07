// Header for the Values

#pragma once

#ifndef VALUES_HPP
#define VALUES_HPP

#include <string>
#include <memory>


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
            Boolean
        };

        Type type;
        float number;
        bool bvalue;

        // to make a Number Value
        static RuntimeVal Null() {
            return RuntimeVal{};
        }

        // to Make a Number Value
        static RuntimeVal Number(float n) {
            return RuntimeVal(static_cast<float>(n));
        }

        // to Make a Boolean Value
        static RuntimeVal Boolean(bool b) {
            return RuntimeVal(static_cast<bool>(b));
        }

        // Null Konstruktor
        RuntimeVal() : type(Type::Null), number(0), bvalue(false) {}

        // float Kontruktor
        RuntimeVal(float n) : type(Type::Number), number(n), bvalue(false) {}

        // Boolean Constuctor
        // This could mabybe lead to errors
        RuntimeVal(bool b) : type(Type::Boolean), number(0), bvalue(b) {}

        // Convert to String
        std::string toString() const
        {
            std::string return_msg = "{ type: ";

            switch (type)
            {

                //      // Null
            case (Type::Null): {
                return_msg += "\"null\"";
                return_msg += ", value: null";
                break;
            }

                             //      // Number
            case (Type::Number): {
                return_msg += "\"number\"";
                return_msg += ", value: " + std::to_string(number);
                break;
            }

                               //      // Boolean
            case (Type::Boolean): {
                return_msg + "\"�boolean\"" + ", value: ";
                if (bvalue == 0)
                {
                    return_msg += "false";
                }
                else
                {
                    return_msg += "true";
                }
                break;
            }
            };

            return_msg += " }";

            return return_msg;
        };
    };
} // namespace fling::runtime

#endif // VALUES_HPP
