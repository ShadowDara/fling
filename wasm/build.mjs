#!/usr/bin/env node
// build.mjs
// Builds the Fling WebAssembly module (compiles the C++ interpreter
// with Emscripten, producing dist/fling.js + dist/fling.wasm).
//
// Usage:
//   node build.mjs                # build release (default)
//   node build.mjs --debug        # build debug variant
//
// Requires Emscripten (emcc) on PATH or EMSDK pointing at an emsdk dir.

import { execFileSync } from "node:child_process";
import { existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { rmSync, mkdirSync } from "node:fs";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const outDir = join(root, "wasm", "dist");

const debug = process.argv.includes("--debug");

// --- locate emcc (use EMSDK if set, else PATH) -------------------------
function findEmcc() {
  const emsdk = process.env.EMSDK;
  if (emsdk) {
    const win = join(emsdk, "upstream/emscripten/emcc.bat");
    if (existsSync(win)) return win;
    const nix = join(emsdk, "upstream/emscripten/emcc");
    if (existsSync(nix)) return nix;
  }
  return "emcc";
}

const emcc = findEmcc();

const sources = [
  join(root, "cpp", "fling.cpp"),
  join(root, "cpp", "util.cpp"),
  join(root, "cpp", "frontend", "ast.cpp"),
  join(root, "cpp", "frontend", "lexer.cpp"),
  join(root, "cpp", "frontend", "parser.cpp"),
  join(root, "cpp", "runtime", "interpreter.cpp"),
  join(root, "cpp", "runtime", "envirments.cpp"),
  join(root, "cpp", "runtime", "eval", "statements.cpp"),
  join(root, "cpp", "runtime", "eval", "expressions.cpp"),
  join(root, "cpp", "wasm", "em.cpp"),
];

const flags = [
  "-std=c++20",
  "-O3" + (debug ? " -g" : ""),
  "--bind", // embind: exposes the API to JS
  "-sMODULARIZE=1", // factory function instead of global Module
  "-sEXPORT_ES6=1", // ES module export
  "-sEXPORT_NAME=createFling",
  "-sENVIRONMENT=web,node", // run in browser and Node.js
  "-sALLOW_MEMORY_GROWTH=1",
  "-sFILESYSTEM=0", // no virtual FS needed (string API only)
  "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap",
  "-sINCOMING_MODULE_JS_API=onRuntimeInitialized,locateFile",
  "-I" + join(root, "cpp"),
  "-o",
  join(outDir, debug ? "fling-debug.js" : "fling.js"),
];

console.log(`[fling-wasm] building ${debug ? "debug" : "release"} ...`);
console.log(`[fling-wasm] emcc: ${emcc}`);

rmSync(outDir, { recursive: true, force: true });
mkdirSync(outDir, { recursive: true });

try {
  const out = execFileSync(emcc, [...sources, ...flags], {
    stdio: "inherit",
  });
  if (out) process.stdout.write(out);
} catch (err) {
  console.error("[fling-wasm] build failed:", err.message);
  process.exit(1);
}

const built = debug ? "fling-debug.js/wasm" : "fling.js/wasm";
console.log(`[fling-wasm] done -> wasm/${built}`);