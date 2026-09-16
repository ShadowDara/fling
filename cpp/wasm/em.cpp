// em.cpp
// Emscripten (Wasm) Binding Layer for Fling.
//
// Exposes a string-based API to JavaScript:
//   - runCode(code)              -> evaluates code, returns nothing
//   - runCodeCapture(code)       -> evaluates code and returns stdout (print output)
//   - createEnv()                -> creates a persistent Environment (returns handle id)
//   - runCodeInEnv(code, envId)  -> evaluates code in an existing Environment
//   - envGet(envId, name)        -> reads a variable from an Environment as string
//   - destroyEnv(envId)          -> frees an Environment
//
// Built with: emcc ... -lembind (see wasm/build.mjs)

#include <emscripten/bind.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "../fling.hpp"

using namespace fling;
using namespace fling::runtime::envirment;

namespace {

// Buffers stdout so that print() output can be returned to JS.
// Emscripten's embind cannot share a C++ std::ostream directly with JS,
// so we capture it in a std::ostringstream instead.
std::ostringstream g_capture;

// Owns all Environments that were created from JS.
std::unordered_map<int, std::shared_ptr<Environment>> g_envs;
int g_next_env = 1;

} // namespace

namespace fling_wasm {

// Evaluate Fling source code. Output goes to stdout (browser console / node stdout).
void runCode(const std::string& code)
{
    fling::runCode(code);
}

// Evaluate Fling source code and return everything that print() wrote
// to stdout during the run.
std::string runCodeCapture(const std::string& code)
{
    std::ostringstream capture;
    std::streambuf* old = std::cout.rdbuf(capture.rdbuf());
    try
    {
        fling::runCode(code);
        std::cout.rdbuf(old);
    }
    catch (...)
    {
        std::cout.rdbuf(old);
        throw;
    }
    return capture.str();
}

// Create a new (persistent) Environment. Returns an id that can be used
// with runCodeInEnv / envGet / destroyEnv.
int createEnv()
{
    int id = g_next_env++;
    g_envs[id] = std::make_shared<Environment>(nullptr);
    return id;
}

// Evaluate Fling source code in a previously created Environment
// (persistent variables between calls).
std::string runCodeInEnv(const std::string& code, int envId)
{
    auto it = g_envs.find(envId);
    if (it == g_envs.end())
    {
        return "Error: invalid environment id " + std::to_string(envId);
    }

    std::ostringstream capture;
    std::streambuf* old = std::cout.rdbuf(capture.rdbuf());
    try
    {
        fling::runCodeInEnvirment(code, it->second);
        std::cout.rdbuf(old);
    }
    catch (...)
    {
        std::cout.rdbuf(old);
        throw;
    }
    return capture.str();
}

// Read a variable from an Environment (stringified via RuntimeVal::toString()).
std::string envGet(int envId, const std::string& name)
{
    auto it = g_envs.find(envId);
    if (it == g_envs.end())
    {
        return "Error: invalid environment id " + std::to_string(envId);
    }

    try
    {
        return it->second->lookupVar(name).toString();
    }
    catch (const std::exception& e)
    {
        return std::string("Error: ") + e.what();
    }
}

// Free an Environment created with createEnv().
void destroyEnv(int envId)
{
    g_envs.erase(envId);
}

} // namespace fling_wasm

EMSCRIPTEN_BINDINGS(fling_wasm)
{
    emscripten::function("runCode", &fling_wasm::runCode);
    emscripten::function("runCodeCapture", &fling_wasm::runCodeCapture);
    emscripten::function("createEnv", &fling_wasm::createEnv);
    emscripten::function("runCodeInEnv", &fling_wasm::runCodeInEnv);
    emscripten::function("envGet", &fling_wasm::envGet);
    emscripten::function("destroyEnv", &fling_wasm::destroyEnv);
}