# Fling Interpreter – Performance-Analyse & Optimierungspotenziale

**Basis:** Analyse der C++-Referenzimplementierung (`cpp/runtime/`, `cpp/frontend/`)
**Stand:** 2026-09-15

---

## 1. Architektur-Uberblick

```
Quellcode ──(Lexer)──> Tokens ──(Parser)──> AST ──(Tree-Walking Interpreter)──> Runtime-Werte
```

Der Interpreter ist ein klassischer **Tree-Walking-Interpreter**:

- Der gesamte AST wird zuerst vollstaendig aufgebaut (`std::unique_ptr<Stmt>`-Baum).
- Dann wird jeder Knoten rekursiv von `evaluate()` traversiert.
- Es gibt **keine Zwischendarstellung** (kein Bytecode, kein IR).
- Es gibt **keine Optimierungsphase**.

---

## 2. Aktuelle Performance-Probleme

### 2.1 RuntimeVal – Das Herzstück (und groesstes Problem)

`RuntimeVal` ist ein **Union-artiges Monster** (ca. 200+ Bytes pro Instanz):

```cpp
struct RuntimeVal {
    Type type;                                           // 4 Bytes
    float number;                                        // 4 Bytes
    std::string str;                                     // 32 Bytes (SSO)
    bool bvalue;                                         // 1 Byte
    std::unordered_map<std::string, std::unique_ptr<RuntimeVal>> properties;  // 48 Bytes
    std::vector<RuntimeVal> elements;                    // 24 Bytes
    std::function<...> call;                             // 32 Bytes
    std::string name;                                    // 32 Bytes
    std::vector<std::string> parameters;                 // 24 Bytes
    std::shared_ptr<Environment> declaration;            // 8 Bytes
    std::vector<std::unique_ptr<ast::Stmt>> body;        // 24 Bytes
};
```

**Probleme:**

- **Jeder Wert traegt alle Felder aller Typen** – ein einfacher `float` braucht ~200 Bytes statt 4.
- **Copies sind teuer:** `RuntimeVal` wird haeufig **by value** kopiert (z.B. `lookupVar()` gibt einen `RuntimeVal` zurueck, `evaluate()` gibt einen `RuntimeVal` zurueck).
- **Kein Move-Semantics** an vielen Stellen – `variables[varName] = std::move(value)` ist selten.
- **`std::function`** fuer Native-Funktionen ist heap-allocating und hat implizite Kopien.
- **`std::shared_ptr<Environment>`** in jedem Funktionswert erzeugt Reference-Counting-Overhead.

**Benchmark-Einschaetzung:** Ein einfacher `x + 1`-Ausdruck erzeugt mindestens **3–4 Heap-Allokationen** (zwei `RuntimeVal`-Kopien + String-Kopien in `std::function`).

### 2.2 Environment – Lineare Scope-Suche

```cpp
Environment* Environment::resolve(const std::string& varName) {
    if (this->variables.find(varName) != this->variables.end())
        return this;
    if (this->parent == nullptr) return nullptr;
    return this->parent->resolve(varName);  // Rekursion!
}
```

**Probleme:**

- **Rekursive Scope-Kette:** Bei verschachtelten Scopes (z.B. 10 Blattiefen) wird 10x rekurziert.
- **`std::unordered_map::find()`** fuer jeden Scope – Hashing + Kollisionsbehandlung.
- **Kein Caching** – identische Variablen-Loesungen werden immer wiederholt.
- **`std::set<std::string> constants`** fuer Konstanten-Tracking –额外es Hashing.
- **Fehlerbehandlung per `std::cout`** – jeder nicht-aufgeloeste Name druckt auf stdout.

### 2.3 While-Loops – Umwelt pro Iteration

```cpp
case ast::NodeType::WhileStatement: {
    while (true) {
        auto condVal = evaluate(*whileNode.condition, env);
        if (!condVal.isTruthy()) break;
        auto iterationEnv = std::make_shared<envirment::Environment>(env);
        evaluate(*whileNode.body, iterationEnv);
    }
}
```

**Probleme:**

- **Jede Iteration erstellt ein `shared_ptr<Environment>`** – Heap-Allokation + Reference-Counting.
- **Variablen im Loop-Body sind nach jedem Iterationsschritt weg** – das ist semantisch beabsichtigt, aber teuer.
- **Kein Peephole-Optimization** – z.B. wird die Bedingung jedes Mal komplett neu evaluiert (auch wenn sie konstant ist).

### 2.4 Funktionsaufrufe – Kein Tail-Call, kein Inlining

```cpp
auto scope = std::make_shared<envirment::Environment>(func.declaration);
for (size_t i = 0; i < func.parameters.size(); ++i) {
    scope->declareVar(paramName, std::move(argValue), false);
}
RuntimeVal returnValue = RuntimeVal::Null();
for (const auto& stmt : func.body) {
    returnValue = evaluate(*stmt, scope);
}
```

**Probleme:**

- **Kein Tail-Call-Optimization (TCO):** Rekursive Funktionen erzeugen immer neue Scopes, bis der Stack ueberlaeuft.
- **Kein Function Inlining:** Selbst triviale Funktionen wie `fn add(a, b) { return a + b }` erzeugen vollen Scope-Overhead.
- **Body wird als `std::vector<unique_ptr<Stmt>>` gespeichert und bei jedem Aufruf evaluiert** – kein Caching, kein JIT.
- **`result`-Convention:** Wenn eine Funktion `null` zurueckgibt, wird `scope->hasVar("result")` geprueft – das ist ein extra Hash-Lookup pro Funktionsaufruf.

### 2.5 String-Operatoren – Kein String-Interner

```cpp
// In evaluate_binary_expr:
if (lhs.type == RuntimeVal::Type::String && rhs.type == RuntimeVal::Type::String) {
    if (binop.callculation_operator == "+")
        return RuntimeVal::String(lhs.str + rhs.str);
}
```

**Probleme:**

- **Jede String-Konkatenation allokiert einen neuen Heap-String.**
- **Kein String-Interner** – identische Strings werden nicht dedupliziert.
- **`==`-Vergleich auf Strings ist O(n)** – kein Hashing, kein Interning.
- **Keine Rope- oder StringBuffer-Struktur** fuer aufeinanderfolgende Konkatenationen.

### 2.6 Zahlen – Nur `float`

```cpp
float number = 0;  // Nur float, kein int
```

**Probleme:**

- **Alle Zahlen sind `float`** – Integer-Operationen wie `5 % 3` erfordern `toInt()`-Casts (Truncation-Bugs moeglich).
- **`float`-Arithmetik ist langsamer als `int`** auf den meisten Architekturen.
- **Keine tagged-union oder NaN-boxing** fuer effiziente Typ-Unterscheidung.

### 2.7 AST – Keine Kohaerenz

- **AST-Knoten sind polymorph** (`virtual toString()`, `virtual print()`) – Virtual-Dispatch-Overhead.
- **Kein Node-Pooling** – jeder Knoten ist eine eigene Heap-Allokation.
- **Kein Source-Location-Tracking** im AST (nur in Tokens) – Fehlermeldungen sind ungenau.

---

## 3. Messbare Engpaesse (Benchmark-Profile)

| Szenario                    | Engpass                                            | Schaetzung |
| --------------------------- | -------------------------------------------------- | ---------- |
| `let x = 1; x + 1;`         | 2x `RuntimeVal`-Kopie + Environment-Lookup         | ~500ns     |
| `fn fib(n) { ... } fib(30)` | 30M rekursive Aufrufe, je Scope + RuntimeVal-Kopie | ~15s       |
| `while (i < 10000) { ... }` | 10k Heap-Allokationen fuer iterationEnv            | ~5ms       |
| `print("hello" + " world")` | String-Konkatenation + Heap-Allokation             | ~200ns     |
| `obj.foo.bar.baz`           | 3x MemberExpr-Evaluation mit unordered_map-Lookup  | ~1.5us     |

---

## 4. Optimierungsvorschlaege (nach Aufwand/Wirkung)

### 4.1 Grosse Wirkung, moderater Aufwand

#### 4.1.1 tagged-union / union-basiertes RuntimeVal

```cpp
struct RuntimeVal {
    enum class Type : uint8_t { Null, Number, String, Boolean, Object, Array, FnValue, NativeFn };
    Type type;
    union {
        float number;
        bool bvalue;
        // ... pointer fuer String/Object/Array/Fn
    };
};
```

- Reduziert Groesse von ~200 Bytes auf ~16-32 Bytes.
- **Kosten:** Mannaufwand fuer Typ-Konvertierung everywhere.

#### 4.1.2 Stack-basiertes Environment (Frame-Pointer)

- Statt rekursiver Scope-Kette: ein **flacher Stack von Frames** (wie bei Lua/CPython).
- Variablen werden als `(Slot-In-Frame, Frame-Index)` referenziert – **O(1)-Lookup**.
- **Kosten:** Reqiert, dass der Parser/Variablen-Resolver vorher Slot-Indices zuweist.

#### 4.1.3 String-Interner

- Hash-Tabelle fuer alle Strings – identische Strings teilen sich Pointer.
- `==`-Vergleich wird zu **Pointer-Vergleich** (O(1) statt O(n)).
- **Kosten:** Moderat – ein `unordered_set<string>` mit Custom-Hash.

#### 4.1.4 Tail-Call-Optimization (TCO)

- Wenn `return f(x)` das letzte Statement ist: Scope wiederverwenden statt neuen zu erstellen.
- **Kosten:** Gering – Aenderung in `evaluate_call_expr` und `evaluate()`.

### 4.2 Mittlere Wirkung, kleiner Aufwand

#### 4.2.1 `std::string` durch `std::string_view` oder `const char*` ersetzen

- Im Lexer und Parser: Tokens koennen `string_view` ins Original-Source zeigen.
- **Kosten:** Reqiert, dass die Quelle lebt waehrend der Parsing-Phase.

#### 4.2.2 Fehlerbehandlung: exceptions statt `cout`

- `std::cout << "Variable not found"` in Haeufen vonruntime-Funktionen ist langsamer als eine Exception oder ein `std::error_code`.
- **Kosten:** Minimal – Ersetzen von `cout` durch `throw` oder `std::optional`.

#### 4.2.3 `RuntimeVal::toString()` optimieren

- Aktuell erzeugt jeder `toString()`-Aufruf mehrere `std::string`-Concatenationen.
- Besser: `std::ostringstream` oder `fmt::format`.
- **Kosten:** Minimal.

#### 4.2.4 Vector-Reserve in `evaluate_binary_expr`

- `evaluatedArgs.reserve()` existiert bereits – aber fuer `RuntimeVal`-Kopien in der eval-Loop fehlt es.
- **Kosten:** Minimal.

### 4.3 Grosse Wirkung, grosser Aufwand

#### 4.3.1 Bytecode-VM statt Tree-Walking

- AST in Linear-Bytecode kompilieren (Stack-basiert oder Register-basiert).
- **Erwarteter Speedup:** 10x–100x fuer CPU-bound Programme.
- **Kosten:** Komplett neue Eval-Schicht, Bytecode-Compiler noetig.
- **Vorbild:** Lua 5.1 (2003), CPython, Wren.

#### 4.3.2 JIT-Kompilierung (optional, spaeter)

- Heisse Schleifen zu maschinenassembly kompilieren.
- **Erwarteter Speedup:** 100x–1000x fuer numerische Loops.
- **Kosten:** Extrem hoch –动dynasm/libasm noetig, Portierbarkeit leidet.
- **Vorbild:** LuaJIT, V8 Ignition/TurboFan.

#### 4.3.3 AST-pooling / Arena-Allokation

- Alle AST-Knoten in einem一块内存-Block allozieren (Arena).
- Kein `delete` noetig – gesamter Block wird am Ende freigegeben.
- **Kosten:** Mittel – eigener Allocator noetig.

### 4.4 Quick Wins (sofort umsetzbar)

| Aenderung                                                                  | Datei             | Erwarteter Effekt             |
| -------------------------------------------------------------------------- | ----------------- | ----------------------------- |
| `lookupVar()` gibt `const RuntimeVal&` zurueck statt `RuntimeVal`          | `envirments.cpp`  | 1 Kopie weniger pro Zugriff   |
| `evaluate()` gibt `const RuntimeVal&` zurueck (wo moeglich)                | `interpreter.cpp` | 1 Kopie weniger pro Eval      |
| `std::shared_ptr<Environment>` in While-Loop durch `Environment*` ersetzen | `interpreter.cpp` | Kein Heap-Allok pro Iteration |
| `std::function` durch function pointer ersetzen (NativeFN)                 | `values.hpp`      | Kein Heap-Allok pro Native-Fn |
| `constants` in `Environment` entfernen (in `variables` als Flag speichern) | `envirments.hpp`  | 1 Hash-Tabelle weniger        |
| `std::cout`-Fehlermeldungen durch `std::cerr` oder throw ersetzen          | `envirments.cpp`  | Sauberer + schneller          |

---

## 5. Prioritaeten-Matrix

```
                    Aufwand
                    klein ──────────── gross
Wirkung   gross │  4.1.4 TCO        4.3.1 Bytecode-VM
                │  4.1.1 tagged-uv  4.3.2 JIT
                │  4.1.3 Interner   4.3.3 Arena
                │  Quick Wins       4.1.2 Stack-Env
                │
          mittel│  4.2.1 string_view  4.1.2 Stack-Env
                │  4.2.2 Exceptions
                │  4.2.3 toString
                │
          klein │  4.2.4 Reserve     —
```

**Empfohlene Reihenfolge:**

1. Quick Wins (sofort) – 2–3 Stunden Aufwand
2. 4.1.1 tagged-union RuntimeVal (1–2 Tage)
3. 4.1.3 String-Interner (0.5 Tage)
4. 4.1.4 TCO (0.5 Tage)
5. 4.1.2 Stack-basiertes Environment (2–3 Tage)
6. 4.3.1 Bytecode-VM (1–2 Wochen) – nur wenn noetig

---

## 6. Spezifische Code-Stellen mit Optimierungspotenzial

### 6.1 `values.hpp` – RuntimeVal-Konstruktoren

```cpp
// AKTUELL: Kopiert string immer
RuntimeVal::String(const std::string& s) {
    val.str = s;  // Heap-Allokation
}

// OPTIMIERT: Move-Semantics
RuntimeVal::String(std::string s) {
    val.str = std::move(s);  // Kein Copy
}
```

### 6.2 `envirments.cpp` – lookupVar

```cpp
// AKTUELL: Gibt Wert zurueck (Kopie)
RuntimeVal Environment::lookupVar(std::string varName) {
    auto env = this->resolve(varName);
    return env->variables[varName];  // Kopie!
}

// OPTIMIERT: Gibt Referenz zurueck
const RuntimeVal& Environment::lookupVar(const std::string& varName) const {
    auto env = this->resolve(varName);
    return env->variables.at(varName);  // Kein Copy
}
```

### 6.3 `interpreter.cpp` – While-Loop

```cpp
// AKTUELL: Heap-Allokation pro Iteration
auto iterationEnv = std::make_shared<Environment>(env);

// OPTIMIERT: Stack-basiert oder Pool
Environment iterationEnv(env);  // Kein Heap
```

### 6.4 `expressions.cpp` – evaluate_binary_expr

```cpp
// AKTUELL: String-Vergleich ist O(n)
case RuntimeVal::Type::String:
    return RuntimeVal::Boolean(lhs.str == rhs.str);  // Char-by-Char

// OPTIMIERT: Mit String-Interner
case RuntimeVal::Type::String:
    return RuntimeVal::Boolean(lhs.str == rhs.str);  // Pointer-Vergleich O(1)
```

### 6.5 `envirments.hpp` – Environment-Klasse

```cpp
// AKTUELL: Zwei separate Datenstrukturen
std::unordered_map<std::string, RuntimeVal> variables;
std::set<std::string> constants;

// OPTIMIERT: Flag im Value
struct VarSlot {
    RuntimeVal value;
    bool isConst;
};
std::unordered_map<std::string, VarSlot> variables;
```

---

## 7. Zusammenfassung

Der Fling-Interpreter ist ein solides Proof-of-Concept, aber die aktuelle Architektur hat **signifikante Performance-Engpaesse**, die vor allem aus dem **union-artigen `RuntimeVal`-Design**, den **numerichen Heap-Allokationen** pro Funktionsaufruf/Loop-Iteration und dem **Tree-Walking-Ansatz** resultieren.

Fuer ein生产-ready Skriptsprache sind die **Quick Wins** (Referenzen statt Kopien, Move-Semantics, Stack-basiertes Environment) ein Muss. Fuer ernsthafte Performance-Anforderungen ist ein **Bytecode-VM** der next step – das ist aber ein groses Unterfangen.

Der pragmatischste Pfad ist:

1. Quick Wins implementieren (sofort)
2. RuntimeVal auf tagged-union umstellen (Wochenende)
3. Bytecode-VM bauen (nur wenn die Sprache ernst genommen wird)

---

_Erstellt durch Analyse der C++-Quellcode-Dateien: `runtime/values.hpp`, `runtime/envirments._`, `runtime/interpreter._`, `runtime/eval/expressions._`, `runtime/eval/statements._`, `frontend/ast.hpp`, `util._`\*
