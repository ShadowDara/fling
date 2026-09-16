// index.d.ts
// TypeScript definitions for the Fling Wasm npm package.

export interface FlingOptions {
  onRuntimeInitialized?: () => void;
  locateFile?: (path: string, prefix: string) => string;
}

export class Fling {
  /** Asynchronously create the Fling Wasm instance. */
  static create(options?: FlingOptions): Promise<Fling>;
  /** Alias for create(). */
  static init(): Promise<Fling>;

  /** Evaluate Fling source code. print() output goes to the console. */
  run(code: string): void;
  /** Evaluate Fling source code and return captured print() output. */
  runCapture(code: string): string;

  /** Create a persistent environment that can be reused across calls. */
  createEnv(): number;
  /** Evaluate code in a persistent environment. Returns captured output. */
  runInEnv(code: string, envId: number): string;
  /** Read a variable from an environment (stringified). */
  envGet(envId: number, name: string): string;
  /** Free a persistent environment. */
  destroyEnv(envId: number): void;
}

export default Fling;
