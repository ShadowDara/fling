// stress.mjs – harte Randfälle für den Fling-Wasm-Parser
import { Fling } from "./index.js";
import { writeFileSync } from "node:fs";

const results = [];
function check(name, actual, expected) {
  const ok = String(actual).includes(String(expected));
  results.push([name, ok, actual, expected]);
  console.log(`${ok ? "PASS" : "FAIL"}  ${name}`);
  if (!ok) {
    console.log(`   got:      ${JSON.stringify(actual)}`);
    console.log(`   expected: ${JSON.stringify(expected)}`);
  }
}

const fling = await Fling.create();
const fails = [];

// leeres print-Argument (parse_agrs leere Liste)
check("print() no args", fling.runCapture("print()"), "");

// print mit mehreren Argumenten (Number + String)
check("print multi args", fling.runCapture(`print(1, "two", 3)`), "1 two 3");

// logische + Vergleichs-Operatoren
check("logical &&", fling.runCapture(`print(true && false)`), "false");
check("logical ||", fling.runCapture(`print(false || true)`), "true");
check("comparison ==", fling.runCapture(`print(5 == 5)`), "true");

// verschachtelte Zuweisung
check(
  "nested assign",
  fling.runCapture(`
let a = 0;
let b = 1;
a = b = 99
print(a)
print(b)
`),
  "99",
);

// Member auf Variable + '.'-Behandlung
check(
  "member plus assign",
  fling.runCapture(`
const obj = { x: 100, y: 32 };
print(obj.x)
`),
  "100",
);

// Object-Literal mit Shorthand
check(
  "object shorthand",
  fling.runCapture(`
const x = 7;
print( x )
`),
  "7",
);

// Array + compar+
check(
  "array indexing",
  fling.runCapture(`
const list = [10, 20, 30];
print(list[1])
`),
  "20",
);

// String-Verkettung
check("string concat", fling.runCapture(`print("abc" + "def")`), "abcdef");

console.log("\n---");
const failed = results.filter((r) => !r[1]);
if (failed.length) {
  console.log(`${failed.length}/${results.length} failed`);
  process.exit(1);
}
console.log(`All ${results.length} passed ✔`);
