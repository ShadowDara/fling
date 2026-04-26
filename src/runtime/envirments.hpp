// Envirment for the Language

#pragma once

#ifndef ENVIRMENT_H
#define ENVIRMENT_H

#include <unordered_map>
#include <string>
#include <set>
#include <stdexcept>
#include <memory>

#include "values.hpp"

#include "dcorelib/dcorelib.h"


namespace fling::runtime::envirment {
    class Environment;  // Forward Declaration der Klasse

    // Function to setup the Standard Envirment for the Language
    void setupStandardEnvironment(Environment& env);

    class Environment {
    private:
        Environment* parent = nullptr;  // optional parent (nullptr if none)
        std::unordered_map<std::string, fling::runtime::RuntimeVal> variables;
		std::set<std::string> constants; // Set to track constant variables

    public:
        // Constructor with optional parent
        explicit Environment(Environment* parentEnv = nullptr)
            : parent(parentEnv)
        {
            if (!parent)
            {
                setupStandardEnvironment(*this);
            }
        }

        // Function to declare a Variable
        fling::runtime::RuntimeVal declareVar(std::string varName,
            fling::runtime::RuntimeVal value, bool constant);

        // Function to Assign a Variable
        fling::runtime::RuntimeVal assignVar(std::string varName,
            fling::runtime::RuntimeVal value);

        // Function to get the Content of a Variable
        fling::runtime::RuntimeVal lookupVar(std::string varName);

        // Function to check if the current Envirment has this Variable
        Environment* resolve(const std::string& varName);

        // Function to check whether a variable exists in this scope or any parent
        bool hasVar(const std::string& varName) const;


        // Delete Copy Constructor and Assignment Operator
        Environment(const Environment&) = delete;
        Environment& operator=(const Environment&) = delete;

        Environment(Environment&&) = default;
        Environment& operator=(Environment&&) = default;
    };
}

#endif //ENVIRMENT_H
