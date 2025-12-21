// Header File for the Runtime Values

#pragma once

#ifndef VALUES_HPP
#define VALUES_HPP

#include <string>

namespace fling
{
    namespace runtime
    {

        // Enum with Runtime Values
        enum class ValueType
        {
            Null,
            Number,
        };

        // A Value Struct for the Runtime
        struct RuntimeVal
        {
            ValueType type;

            int numberValue = 0;
            std::string stringValue;

            RuntimeVal() : type(ValueType::Null) {} // Standard auf Null
            RuntimeVal(ValueType t) : type(t) {}
            virtual ~RuntimeVal() = default;

            // toString method
            virtual std::string toString() const
            {
                return "<RuntimeVal>";
            }
        };

        // a Null struct for the Runtime
        struct NullVal : RuntimeVal
        {
            NullVal() : RuntimeVal(ValueType::Null) {}

            std::string toString() const override
            {
                return "null";
            }
        };

        // a Number struct for the Runtime
        struct NumberVal : RuntimeVal
        {
            int value;

            NumberVal(int v) : RuntimeVal(ValueType::Number), value(v) {}

            std::string toString() const override
            {
                return std::to_string(value);
            }
        };

    } // namespace runtime
} // namespace fling

#endif // VALUES_HPP
