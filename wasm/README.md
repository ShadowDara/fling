# Fling for WebAssembly (npm package)

This directory builds the **Fling C++ interpreter** to WebAssembly with
[Emscripten](https://emscripten.org/) and packages it as a portable npm module
(`fling-lang`).

## Why this approach

Fling is a C++ interpreter. Instead of pulling the Rust/cxx bridge into Wasm,
this package compiles the C++ sources directly with `emcc` and exposes a small,
string-based API:

| Function                  | Description                                                        |
| ------------------------- | ------------------------------------------------------------------ |
| `run(code)`               | Evaluate Fling source. `print()` goes to console/stdout.           |
| `runCapture(code)`        | Evaluate Fling source and return captured `print()` output.        |
| `createEnv()`             | Create a persistent Environment (shared variables).                |
| `runInEnv(code, envId)`   | Evaluate code in a persistent Environment.                         |
| `envGet(envId, name)`     | Read a variable from an Environment (stringified).                 |
| `destroyEnv(envId)`       | Free an Environment.                                               |

## Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
  (`emcc` on `PATH`, or the `EMSDK` environment variable pointing at the SDK).
  Your machine already has it at `D:\emsdk` (v6.0.2).
- Node.js >= 18.

## Build

```sh
node build.mjs          # release (dist/fling.js + dist/fling.wasm)
node build.mjs --debug  # debug build
```

The script locates `emcc` via `EMSDK` (falls back to `PATH`).

## Test

```sh
node test.mjs
```

Runs a smoke test: numbers, functions, closures, variables, objects and
persistent environments.

## Usage (Node.js, ES module)

```js
import { Fling } from "fling-lang";

const fling = await Fling.create();

const output = fling.runCapture(`
  fn add(a, b) { a + b }
  print(add(2, 3))
`);
console.log(output); // "5\n"

// persistent environment
const envId = fling.createEnv();
fling.runInEnv(`let count = 0;`, envId);
fling.runInEnv(`count = count + 1; print(count)`, envId); // "1"
```

## Usage (browser)

```html
<script type="module">
  import { Fling } from "fling-lang";
  const fling = await Fling.create();
  fling.runCapture(`print("hello from fling")`).then(console.log);
</script>
```

> Note: `runCapture` is synchronous (Wasm runs on the main thread). For large
> scripts this can block the UI briefly – fine for scripting, not for
> interactive editors.

## Publishing to npm

```sh
npm login
npm publish
```

`package.json` already ships only `index.js`, `index.d.ts`, `dist/*`, and the
README. If you want a scoped package, change `"name"` to `@you/fling-lang` and
publish with `npm publish --access public`.

## Troubleshooting

- **`emcc: command not found`** – set `EMSDK` to your emsdk dir, e.g.
  `$env:EMSDK = "D:\emsdk"` in PowerShell, then re-run `node build.mjs`.
- **Wasm not loading in Node** – Emscripten 6 emits ES modules; Node >= 18
  handles them fine.
- **Print output missing** – `print()` writes to `stdout`. Use `runCapture()`
  to get it as a string.

## Alternative: plain CMake (optional)

If you prefer a CMake-based flow over `build.mjs`, `fling-wasm.cpp` shows the
same API as a standalone target that can be added to a CMake project.

## How it works

- `cpp/wasm/em.cpp` – embind bindings (`EMSCRIPTEN_BINDINGS`) over the
  existing `fling::runCode` / `fling::runCodeInEnvirment` API.
- `build.mjs` – invokes `emcc` with `--bind`, `-sMODULARIZE=1`,
  `-sEXPORT_ES6=1`, `-sENVIRONMENT=web,node`, `-sFILESYSTEM=0`,
  `-sALLOW_MEMORY_GROWTH=1`.
- `index.js` – lazy factory that loads `dist/fling.js` once and exposes the
  `Fling` class.