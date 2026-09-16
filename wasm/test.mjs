// test.mjs
// Smoke test for the Fling Wasm module (run with: node test.mjs)

import { Fling } from "./index.js";

const results = [];
function check(name, actual, expected) {
  const ok = actual.includes(expected);
  results.push({ name, ok, actual, expected });
  console.log(`${ok ? "PASS" : "FAIL"}  ${name}`);
  if (!ok) {
    console.log(`      expected to contain: ${JSON.stringify(expected)}`);
    console.log(`      got: ${JSON.stringify(actual)}`);
  }
}

const fling = await Fling.create();

// 1) basic evaluation with print
const out1 = fling.runCapture(`print(45)`);
check("print(45)", out1.trim(), "45");

// 2) functions
const out2 = fling.runCapture(`
fn add(x, y) {
  let result = x + y;
  print(result)
  result
}
print(add(10, 10))
`);
check("add(10, 10)", out2, "20");

// 3) closures
const out3 = fling.runCapture(`
fn makeAdder (offset) {
  fn add (x, y) {
    x + y + offset
  }
  add
}
const adder = makeAdder(1);
print(adder(10, 5))
`);
check("closure adder(10, 5)", out3.trim(), "16");

// 4) constants / variables
const out4 = fling.runCapture(`let foo = 50;
foo = foo / 2
print(foo)`);
check("let foo = 50 / 2", out4.trim(), "25");

// 5) persistent environment
const envId = fling.createEnv();
fling.runInEnv(`let counter = 1;`, envId);

check("env first run", "", "");

const outEnv = fling.runInEnv(
  `counter = counter + 1
print(counter)`,
  envId,
);
check("env counter after inc", outEnv.trim(), "2");
const outGet = fling.envGet(envId, "counter");
check("envGet counter", outGet, '"number"');
fling.destroyEnv(envId);

// 6) objects
const out6 = fling.runCapture(`
const obj = {
  x: 100,
  y: 32,
};
print(obj.x)
`);
check("object property", out6.trim(), "100");

console.log("\n---");
const failed = results.filter((r) => !r.ok);
if (failed.length) {
  console.log(`${failed.length}/${results.length} tests failed`);
  process.exit(1);
} else {
  console.log(`All ${results.length} tests passed ✔`);
}
