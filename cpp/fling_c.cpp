#include "fling.h"

// fling C

#include <string>

#include "fling.hpp"

// run Fling Code from C
void runCodeC(const char* code)
{
	// Call the C++ function to run the code
	fling::runCode(std::string(code));
}
