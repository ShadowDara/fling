// Envirment for the Language

#pragma once

#ifndef ENVIRMENT_H
#define ENVIRMENT_H

#include <unordered_map>
#include <string>
#include <set>
#include <stdexcept>

#include "values.hpp"

#include "dcorelib/dcorelib.h"


namespace fling::runtime::envirment {
    class Environment;  // Forward Declaration der Klasse

    // Function to setup the Standard Envirment for the Language
    void setupStandardEnvironment(envirment::Environment* env);

    class Environment {
    private:
        Environment* parent;  // optional parent (nullptr if none)
        std::unordered_map<std::string, fling::runtime::RuntimeVal> variables;
		std::set<std::string> constants; // Set to track constant variables

    public:
        // Constructor with optional parent
        explicit Environment(Environment* parentEnv = nullptr)
            : parent(parentEnv), variables(), constants()
        {
			auto global_env = parentEnv == nullptr ? this : parentEnv;

            if (global_env)
            {
				// Setup the standard environment only for the global environment
                setupStandardEnvironment(this);
			}
        }

        // Function to declare a Variable
        fling::runtime::RuntimeVal declareVar(std::string varName,
            fling::runtime::RuntimeVal value, bool constant);

        // Function to Assign a Variable
        fling::runtime::RuntimeVal assignVar(std::string varName,
            fling::runtime::RuntimeVal value);

        // Function to get the Content of a Variable
        fling::runtime::RuntimeVal lookupVar(std::string);

        // Function to check if the current Envirment has this Variable
        Environment* resolve(std::string varName);
    };
}

#endif //ENVIRMENT_H
