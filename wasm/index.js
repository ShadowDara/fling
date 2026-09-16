// index.js
// npm package entry point for the Fling interpreter compiled to WebAssembly.
//
// Usage:
//   import { Fling } from "fling-lang";
//   const fling = await Fling.create();
//   fling.run(`print("hello from fling!")`);

import createFling from "./dist/fling.js";

let modulePromise = null;

function loadModule() {
  if (!modulePromise) {
    modulePromise = createFling();
  }
  return modulePromise;
}

async function extractResult(module, result) {
  if (typeof result === "string") return result;
  return module.UTF8ToString(result);
}

export class Fling {
  constructor(module) {
    this._module = module;
  }

  /**
   * Creates a new Fling (Wasm) instance.
   * @param {object} [options] - optional Emscripten module options
   * @returns {Promise<Fling>}
   */
  static async create(options) {
    const module = await loadModule();
    return new Fling(module);
  }

  /**
   * Same as `create()` – keeps the async contract simple.
   * @returns {Promise<Fling>}
   */
  static async init() {
    return Fling.create();
  }

  /**
   * Evaluates Fling source code. print() output goes to the console
   * (browser or Node.js stdout).
   * @param {string} code
   */
  run(code) {
    this._module.runCode(code);
  }

  /**
   * Evaluates Fling source code and returns everything that print()
   * wrote during the run, as a string.
   * @param {string} code
   * @returns {string}
   */
  runCapture(code) {
    return this._module.runCodeCapture(code);
  }

  /**
   * Creates a persistent environment. Multiple runInEnv() calls share
   * the same variable scope.
   * @returns {number} environment id
   */
  createEnv() {
    return this._module.createEnv();
  }

  /**
   * Evaluates Fling source code inside a persistent environment.
   * @param {string} code
   * @param {number} envId
   * @returns {string} captured print output
   */
  runInEnv(code, envId) {
    return this._module.runCodeInEnv(code, envId);
  }

  /**
   * Reads a variable from a persistent environment (stringified).
   * @param {number} envId
   * @param {string} name
   * @returns {string}
   */
  envGet(envId, name) {
    return this._module.envGet(envId, name);
  }

  /**
   * Frees a persistent environment.
   * @param {number} envId
   */
  destroyEnv(envId) {
    this._module.destroyEnv(envId);
  }
}

export default Fling;
