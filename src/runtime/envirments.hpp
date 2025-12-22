// Envirment for the Language

#pragma once

#ifndef ENVIRMENT_H
#define ENVIRMENT_H

#include <unordered_map>
#include <string>
#include <stdexcept>

#include "values.hpp"

#include "dcorelib/dcorelib.h"

namespace fling::runtime::envirment {

    class Environment {
    private:
        Environment* parent;  // optional parent (nullptr if none)
        std::unordered_map<std::string, fling::runtime::RuntimeVal> variables;

    public:
        // Constructor with optional parent
        explicit Environment(Environment* parentEnv = nullptr)
            : parent(parentEnv), variables() {
        }

        // Function to declare a Variable
        fling::runtime::RuntimeVal declareVar(std::string varName, fling::runtime::RuntimeVal value);

        // Function to Assign a Variable
        fling::runtime::RuntimeVal assignVar(std::string varName, fling::runtime::RuntimeVal value);

        // Function to get the Content of a Variable
        fling::runtime::RuntimeVal lookupVar(std::string);

        // Function to check if the current Envirment has this Variable
        Environment* resolve(std::string varName);
    };
}

#endif //ENVIRMENT_H
