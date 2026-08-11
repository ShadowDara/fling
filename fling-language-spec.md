# Fling – Sprachspezifikation & Feature-Dokument
### Grundlage für die Entwicklung eines Language Server Protocol (LSP)

**Quelle:** Analyse des vollständigen C++-Referenzinterpreters (`cpp.zip`)
**Sprachname im Code:** `fling` (Namespace `fling::lexer`, `fling::ast`, `fling::parser`, `fling::runtime`)
**Analysierte Module:** `frontend/lexer.*`, `frontend/ast.*`, `frontend/parser.*`, `runtime/*`, `runtime/eval/*`, `rustffi/ffi.*`, `util.*`

---

## 1. Überblick

Fling ist eine kleine, dynamisch typisierte, interpretierte Skriptsprache mit C-/JavaScript-artiger Syntax (geschweifte Klammern, `let`/`const`, Objekt- und Array-Literale). Die Referenzimplementierung ist ein klassischer Tree-Walking-Interpreter in drei Phasen:

```
Quellcode ──(Lexer)──> Tokens ──(Parser)──> AST ──(Interpreter)──> Runtime-Werte
```

Die C++-Implementierung wird über eine **Rust-FFI-Bridge** (`rustffi/ffi.cpp`, nutzt `rust/cxx.h`) eingebunden. Es gibt eine Funktion `run_file(rust::Str filename)` und einen `runREPL()`-Modus – das deutet darauf hin, dass ein Rust-Host (z. B. eine CLI oder ggf. später der LSP-Server selbst) den C++-Kern aufruft. Für ein LSP ist relevant: **Es existiert bereits eine Rust-Anbindung**, die als Basis für einen Rust-basierten Sprachserver (z. B. mit `tower-lsp`) dienen könnte, indem Lexer/Parser/AST wiederverwendet oder nachgebaut werden.

⚠️ Hinweis: Der Code enthält mehrere **Bugs und Inkonsistenzen** gegenüber dem vermutlich beabsichtigten Verhalten. Diese sind in Abschnitt 9 gesondert aufgeführt, da sie für Diagnostics/Hover-Texte eines LSP wichtig sind (der LSP sollte im Zweifel das *tatsächliche* Verhalten des Interpreters widerspiegeln, nicht das intuitiv erwartete).

---

## 2. Lexikalische Grammatik (Lexer)

### 2.1 Zeichensätze & Whitespace
- Leerzeichen (` `) und Tabs (`\t`) sind **skippable** (keine Tokens, keine Bedeutung).
- Zeilenumbrüche (`\n`, `\r`) erhöhen den `line`-Zähler und setzen `column` zurück – **keine automatische Semikolon-Einfügung (ASI)**. Zeilenumbrüche haben keinerlei syntaktische Bedeutung.
- Jedes Token trägt `line` und `column` für Fehlermeldungen/Diagnostics.

### 2.2 Kommentare
| Syntax | Typ |
|---|---|
| `# ...` bis Zeilenende | Line-Kommentar |
| `// ...` bis Zeilenende | Line-Kommentar |

Es gibt **keine Blockkommentare** (`/* ... */` wird nicht unterstützt – `/` wird nur als Divisionsoperator oder `//`-Kommentar erkannt).

### 2.3 Keywords (reserviert)
```
let  const  fn  if  else  while
```
Das sind **die einzigen 6 reservierten Schlüsselwörter**. Es gibt **kein** `return`, `for`, `break`, `continue`, `elif`, `switch`, `class`, `struct`, `import`, `null`, `true`, `false` als Keyword.

> **Wichtig für LSP:** `true`, `false`, `null` sind **keine Keywords**, sondern normale Identifier, die im globalen Environment als konstante Variablen vordeklariert werden (siehe 6.7). Sie können in verschachtelten Scopes überschattet ("shadowed") werden.

### 2.4 Identifier
- Regel: `[Alpha][AlphaNumeric]*`
- **Erstes Zeichen muss ein Buchstabe sein** (`std::isalpha`), danach Buchstaben oder Ziffern.
- ⚠️ **Unterstriche `_` sind NICHT erlaubt** – weder am Anfang noch im weiteren Verlauf des Identifiers (kein `isalnum`-Fallback für `_`). `snake_case`-Namen wie `my_var` sind ungültig (`_` erzeugt "Unrecognized character").
- Keine Unicode-Unterstützung (nur ASCII, `std::isalpha`/`std::isalnum` mit `unsigned char`-Cast).

### 2.5 Literale

**Zahlen (`Number`):**
- Nur **Ziffernfolgen** werden vom Lexer erkannt: `while (isInt(src[i])) num += src[i];`
- ⚠️ **Es gibt keine Dezimalpunkt-Erkennung im Lexer.** Obwohl der Parser eine Funktion `parse_float()` besitzt und Werte intern als `float` gespeichert werden, tokenisiert `3.14` als `Number("3")`, `Dot`, `Number("14")` – **keine Fließkommaliterale möglich.** Der Ausdruck `3.14` führt beim Parsen zu einem Parser-Fehler (`exit(1)`), da nach einem `.` laut Parser ein Identifier folgen muss (für Member-Zugriff), nicht eine Zahl.
- Keine negativen Zahlenliterale (negative Zahlen entstehen nur über den unären `-`-Operator).
- Keine Hex-/Oktal-/Binär-Literale, kein `_` als Tausendertrennzeichen, keine Exponentialschreibweise (`1e10`).

**Strings (`String`):**
- Begrenzt durch `"` ... `"`.
- ⚠️ **Keine Escape-Sequenzen.** Es gibt keinerlei Backslash-Behandlung im Lexer. `\"`, `\n`, `\t` etc. werden nicht unterstützt; ein `\` wird als normales Zeichen in den String übernommen. Ein `"` im String beendet ihn immer sofort.
- Nicht geschlossene Strings laufen bis Dateiende (kein Fehler, `i < src.size()`-Check bricht einfach ab).

**Boolean/Null:** Keine eigenen Token-Typen – siehe 2.3/6.7 (`true`/`false`/`null` sind vordeklarierte Identifier).

### 2.6 Operatoren & Interpunktion (Tokentabelle)

| Zeichen(folge) | TokenType | Bedeutung |
|---|---|---|
| `(` `)` | OpenParen / CloseParen | Klammerung, Funktionsargumente |
| `{` `}` | OpenCurlyBrace / CloseCurlyBrace | Blöcke, Objekt-Literale |
| `[` `]` | OpenSquaredBrace / CloseSquaredBrace | Array-Literale, Computed-Member-Access |
| `,` | Comma | Trennzeichen (Args, Array-/Objektelemente) |
| `.` | Dot | Member-Zugriff (nur mit Identifier rechts) |
| `:` | Colon | Key-Value-Trenner in Objekt-Literalen |
| `;` | Semicolon | Nur nach `let`/`const`-Deklarationen Pflicht |
| `=` | Equals | Zuweisung |
| `==` `!=` `<` `>` `<=` `>=` | BinaryOperator | Vergleichsoperatoren |
| `+` `-` `*` `/` `%` | BinaryOperator | Arithmetik (auch unär: `!`, `-`) |
| `!` | BinaryOperator | Logisches NICHT (unär) / Teil von `!=` |
| `&&` `\|\|` | BinaryOperator | Logisches UND/ODER |

⚠️ **Einzelnes `&` oder `|`** (ohne Verdopplung) wird nicht als gültiges Token erkannt – der Lexer gibt `"Unexpected '&'"`/`"Unexpected '|'"` auf `stderr` aus und überspringt das Zeichen still (kein Bitweise-UND/ODER, kein Fehlerabbruch).

Unbekannte Zeichen (z. B. `_`, `$`, `@`, `?`, `^`, `~`) führen dazu, dass `tokenize()` **sofort einen leeren Token-Vektor zurückgibt** und über `stdout` `"Unrecognized character in source"` ausgibt – der gesamte restliche Quelltext wird verworfen.

---

## 3. Grammatik (EBNF) & Operator-Präzedenz

### 3.1 Programmstruktur
```ebnf
Program        := Statement*
Statement       := VarDeclaration
                  | FunctionDeclaration
                  | IfStatement
                  | WhileStatement
                  | ExpressionStatement

Block           := "{" Statement* "}"
```

### 3.2 Deklarationen
```ebnf
VarDeclaration  := ("let" | "const") Identifier ("=" Expression)? ";"
```
- `const` **ohne** Initialisierer ist ein Fehler (Meldung + `nullptr`-Rückgabe, Parser bricht die Deklaration ab).
- `let` ohne Initialisierer ist erlaubt: `let x;` → `x` wird mit `Null` deklariert.
- ⚠️ Semikolon ist **nur nach VarDeclaration Pflicht** – nach Ausdrucks-Statements, `if`, `while`, `fn` **kein** Semikolon (siehe 3.6).

```ebnf
FunctionDeclaration := "fn" Identifier "(" ParamList? ")" "{" Statement* "}"
ParamList       := Identifier ("," Identifier)*
```
- Parameter müssen einfache Identifier sein (keine Default-Werte, kein Rest-Parameter, keine Typannotation, keine Destrukturierung).
- Es gibt **keine anonymen Funktionen/Lambdas** – Funktionen entstehen ausschließlich über `fn`-Deklarationen (die selbst als `const`-artiger Eintrag im Environment landen, siehe 6.5). Funktionswerte können aber als Variablenwerte weitergereicht werden (Funktionen sind "first-class" im Environment, nur die *Syntax* zur Erzeugung ist auf `fn name(...) {...}` beschränkt).

### 3.3 Kontrollfluss
```ebnf
IfStatement     := "if" Expression Block ("else" Block)?
WhileStatement  := "while" Expression Block
```
- Die Bedingung wird **ohne Klammern** geschrieben (`if x > 5 { ... }`, nicht `if (x > 5)`).
- `else if` existiert **nicht** als eigene Grammatikregel – nur `else { if ... { ... } }` (verschachtelt).
- Es gibt **kein** `for`, **kein** `break`/`continue`, **kein** `switch`/`match`.
- Jede `while`-Iteration erzeugt ein neues Kind-Environment (Variablen aus dem Loop-Body sind nach der Iteration nicht mehr sichtbar – "Block Scoping" pro Iteration).

### 3.4 Ausdrücke – Präzedenz (niedrig → hoch bindend)

Aus dem tatsächlichen Aufrufgraphen des Parsers (nicht nur dem im Quellcode dokumentierten Kommentar, der leicht ungenau ist):

| Stufe | Funktion | Operatoren | Assoziativität |
|---|---|---|---|
| 1 (niedrigste) | `parse_assignment_expr` | `=` | rechtsassoziativ |
| 2 | `parse_logical_expr` | `&&`, `\|\|` | linksassoziativ, **gleiche Präzedenz** (kein Vorrang von `&&` vor `\|\|`!) |
| 3 | `parse_comparison_expr` | `==` `!=` `<` `>` `<=` `>=` | linksassoziativ |
| 4 | `parse_additive_expr` | `+` `-` | linksassoziativ |
| 5 | `parse_multiplicitave_expr` | `*` `/` `%` | linksassoziativ |
| 6 | `parse_unary_expr` | `!x`, `-x` (Präfix) | rechtsassoziativ, rekursiv |
| 7 | `parse_call_member_expr` → `parse_member_expr` | `.`, `[...]`, `(...)` (Postfix) | linksassoziativ, siehe 3.5 |
| 8 (höchste) | `parse_primary_expr` | Literale, `(...)`, Identifier, `[...]`, `{...}` | — |

```ebnf
Expression       := AssignmentExpr
AssignmentExpr    := LogicalExpr ("=" AssignmentExpr)?
LogicalExpr       := ComparisonExpr (("&&" | "||") ComparisonExpr)*
ComparisonExpr    := AdditiveExpr (("==" | "!=" | "<" | ">" | "<=" | ">=") AdditiveExpr)*
AdditiveExpr      := MultiplicativeExpr (("+" | "-") MultiplicativeExpr)*
MultiplicativeExpr:= UnaryExpr (("*" | "/" | "%") UnaryExpr)*
UnaryExpr         := ("!" | "-") UnaryExpr | CallMemberExpr
CallMemberExpr    := MemberExpr ( "(" ArgList? ")" )?
MemberExpr        := PrimaryExpr ( "." Identifier | "[" Expression "]" )*
PrimaryExpr       := Identifier | Number | String
                    | "(" Expression ")"
                    | ArrayLiteral
                    | ObjectLiteral
ArrayLiteral      := "[" (Expression ("," Expression)* ","?)? "]"
ObjectLiteral     := "{" (Property ("," Property)* ","?)? "}"
Property          := Identifier (":" Expression)?      // ohne ":" = Shorthand
ArgList           := AssignmentExpr ("," AssignmentExpr)*
```

### 3.5 Wichtige Einschränkungen bei Call-/Member-Verkettung
`parse_call_member_expr` parst **erst** eine vollständige `MemberExpr`-Kette (`.`/`[...]`), **danach höchstens einen** unmittelbar folgenden Funktionsaufruf `(...)`, welcher rekursiv weitere `(...)` erlaubt (`f()()`, Currying-Aufrufe funktionieren). **Nicht unterstützt:**
- `foo().bar` – Member-Zugriff **nach** einem Funktionsaufruf ist **nicht möglich** (Parser stoppt nach dem `CallExpr`, ein folgendes `.` wird zum nächsten Statement/Fehler).
- `foo()[0]` – Computed-Zugriff nach Aufruf ebenfalls nicht möglich.
- Erlaubt ist hingegen: `obj.method()`, `arr[0]()`, `a.b.c.d()` (Member-Kette vor dem Call).

### 3.6 Statement-Terminierung (wichtig für Diagnostics)
| Statement-Typ | Terminator |
|---|---|
| `VarDeclaration` (`let`/`const`) | **Pflicht-Semikolon** |
| `FunctionDeclaration` | kein Semikolon (endet mit `}`) |
| `IfStatement` / `WhileStatement` | kein Semikolon (endet mit `}`) |
| Ausdrucks-Statement (z. B. `print(x)`, `x = 5`) | **kein** Semikolon erwartet/erlaubt |

⚠️ Ein Semikolon nach einem Ausdrucks-Statement (z. B. `print("hi");`) wird vom Parser **nicht konsumiert**. Das `;`-Token bleibt stehen und wird beim nächsten `parse_stmt()`-Aufruf als unerwartetes Token in `parse_primary_expr` behandelt (Fehlermeldung auf `stdout`, Token wird übersprungen, `nullptr` als "Statement" – kein Programmabbruch, aber ein fehlerhafter/verlorener AST-Knoten). **Ein LSP sollte überflüssige Semikola nach Ausdrucks-Statements als Fehler/Warnung markieren.**

---

## 4. AST-Knotentypen (Referenz für Parser-/Hover-Integration)

`NodeType`-Enum (`frontend/ast.hpp`):

**Statements:**
| Node | Felder |
|---|---|
| `Program` | `body: Stmt[]` |
| `VarDeclaration` | `constant: bool`, `identifier: string`, `value: Expr?` |
| `FunctionDeckaration` *(sic, Tippfehler im Enum-Namen)* | `name: string`, `parameters: string[]`, `body: Stmt[]` |
| `IfStatement` | `condition: Expr`, `thenBranch: Stmt`, `elseBranch: Stmt?` |
| `WhileStatement` | `condition: Expr`, `body: Stmt` |

**Ausdrücke:**
| Node | Felder |
|---|---|
| `AssignmentExpr` | `assignme: Expr` (Ziel), `value: Expr` |
| `MemberExpr` | `object: Expr`, `property: Expr`, `computed: bool` |
| `CallExpr` | `caller: Expr`, `agrs: Expr[]` *(Tippfehler: „agrs" statt „args")* |
| `BinaryExpr` | `left: Expr`, `right: Expr`, `callculation_operator: string` *(Tippfehler)* |
| `UnaryExpr` | `op: string` (`"!"` oder `"-"`), `operand: Expr` |
| `Identifier` | `symbol: string` |
| `NumericLiteral` | `value: float` |
| `StringLiteral` | `value: string` |
| `ArrayLiteral` | `elements: Expr[]` |
| `ObjectLiteral` | `properties: Property[]` |
| `Property` | `key: string`, `value: Expr?` |

> Hinweis für LSP-Implementierer: Die Feldnamen im Original-C++ enthalten Tippfehler (`FunctionDeckaration`, `agrs`, `callculation_operator`). Für einen sauberen LSP/eigenen AST empfiehlt es sich, korrekt benannte Äquivalente zu verwenden (`FunctionDeclaration`, `args`, `operator`), die Struktur aber 1:1 zu übernehmen.

---

## 5. Objekt- und Array-Literal-Semantik (Parser-Details)

**Objekt-Literal** `{ key: value, key2, }`:
- Schlüssel müssen **Identifier** sein (keine String-Keys, keine Computed Keys, keine Zahlen als Key).
- **Shorthand-Property** `{ key }` entspricht `{ key: key }` – der Wert wird zur Laufzeit über den gleichnamigen Variablennamen aus dem Environment aufgelöst (nicht zur Parse-Zeit gebunden!).
- Führende/mehrfache Kommata werden vom Parser toleriert (Sonderfall-Handling für optionale Kommata), abschließendes Komma vor `}` ist erlaubt.
- Leeres Objekt `{}` ist gültig.

**Array-Literal** `[1, 2, 3,]`:
- Trailing Comma erlaubt.
- Leeres Array `[]` ist gültig.
- Elemente sind beliebige Ausdrücke (auch verschachtelte Arrays/Objekte).

---

## 6. Laufzeitsemantik (Interpreter)

### 6.1 Werttypen (`RuntimeVal::Type`)
```
Null | Number | String | Boolean | Object | Array | Native_FnValue | FnValue
```
- **Number** ist immer `float` (Single Precision) – es gibt **keinen** separaten Integer-Typ.
- **String**, **Boolean** wie erwartet.
- **Object**: `unordered_map<string, RuntimeVal>` (unsortiert – Iterationsreihenfolge nicht deterministisch/nicht garantiert).
- **Array**: `vector<RuntimeVal>`, homogen oder heterogen möglich (keine Typprüfung).
- **FnValue**: benutzerdefinierte Funktion mit Closure (`declaration`-Environment = definierendes Scope → **lexikalisches Scoping**, Rekursion funktioniert).
- **Native_FnValue**: eingebaute Host-Funktionen (aktuell nur `print`, siehe 7).

### 6.2 Scoping / Environment-Modell
- Jedes `Environment` hat optional einen `parent` (Kette bis zum globalen Root-Environment).
- Variablenauflösung (`resolve`) läuft die Elternkette hoch, bis gefunden oder `nullptr` (Root ohne Treffer).
- **Neue Scopes werden erzeugt bei:** jedem `while`-Loop-Durchlauf (pro Iteration!), jedem Funktionsaufruf (Parameter-Scope, Parent = *Definitions*-Environment der Funktion, nicht der Aufrufer – lexikalisches statt dynamisches Scoping).
- ⚠️ `if`/`else`-Blöcke erzeugen **kein** neues Scope in der aktuellen Implementierung (der `Program`-Block wird im *gleichen* `env` ausgewertet, das übergeben wurde – siehe `evaluate(*ifNode.thenBranch, env)` ohne neues `Environment`). D. h. in `if`/`else`-Blöcken deklarierte Variablen mit `let` sind (bei den meisten Interpretern erwartet man Blockscope) **hier ebenfalls im umgebenden Scope sichtbar bzw. verändern es**, da kein Kind-Environment erzeugt wird. Für ein LSP wichtig: keine Blockscope-Isolation bei `if`/`else`, nur bei `while` und Funktionsaufrufen.

### 6.3 Variablendeklaration (`let`/`const`)
- Redeklaration **im selben Environment** (nicht in Eltern-Scopes) ist ein Fehler: Ausgabe auf `stdout` (`"Can not redeclare Variable!..."`), Rückgabe eines `Null`-Werts, **kein Programmabbruch**.
- `const` erzwingt bei späteren Zuweisungen: `"Cannot assign to constant variable"` (`stdout`, kein Abbruch, Zuweisung wird ignoriert, Ausdruck liefert `Null` statt des neuen Werts).
- Undeklarierte Variablen: Lookup gibt `"Variable not found: <name>"` aus (`stdout`) und liefert `Null` statt eines Fehlers/Exception – **es gibt keine echten Laufzeitfehler/Exceptions**, nur Konsolenausgaben. Für ein LSP bedeutet das: Diagnostics müssen rein **statisch** (durch eigene Analyse) erkannt werden, der Referenzinterpreter selbst "crasht" nicht.

### 6.4 Zuweisung (`AssignmentExpr`)
- **Nur einfache Identifier als Ziel unterstützt** (`x = wert`). `obj.prop = wert` oder `arr[0] = wert` sind **syntaktisch parsebar** (der Parser baut ein `AssignmentExpr` mit `MemberExpr` als Ziel), aber zur Laufzeit fehlerhaft: `evaluate_assignment_expr` castet das Ziel **ungeprüft** auf `Identifier*`, was bei einem `MemberExpr`-Ziel zu **Undefined Behavior** führt (reiner C-Style-Cast auf falschen Typ). **Für das LSP: Zuweisungen an Member-/Index-Ausdrücke sollten als Fehler/nicht unterstütztes Feature markiert werden.**

### 6.5 Funktionsdeklaration & -aufruf
- `fn name(params) { body }` wird im aktuellen Environment als **konstante** Variable (`constant = true`) deklariert, die einen `FnValue` mit Closure enthält.
- **Aufrufsemantik:** Bei jedem Call wird ein neues Environment erzeugt, Parent = das Environment **zur Definitionszeit** der Funktion (Closures funktionieren, echtes lexikalisches Scoping).
- Parameter werden nicht typgeprüft; fehlende Argumente werden mit `Null` aufgefüllt; überzählige Argumente werden ignoriert (keine Arity-Prüfung/-Fehler).
- ⚠️ **Kein `return`-Schlüsselwort.** Der Rückgabewert einer Funktion ist:
  1. der Wert des **zuletzt ausgewerteten Statements** im Funktionskörper, **oder**
  2. falls dieser Wert `Null` ist **und** im Funktions-Scope eine Variable namens **`result`** existiert → wird deren Wert stattdessen zurückgegeben.
  
  Beispiel:
  ```fling
  fn add(a, b) {
      a + b
  }
  ```
  Hier ist `a + b` (als Ausdrucks-Statement ohne Semikolon) der letzte Statement-Wert → wird implizit zurückgegeben.
  ```fling
  fn compute(x) {
      let result = x * 2;
      let unused = 0;
  }
  ```
  Hier ist das letzte Statement `let unused = 0;` (liefert `Null` als Rückgabewert der Deklaration), aber weil `result` im Scope existiert, wird **`result`** zurückgegeben.
- Es gibt **keine vorzeitige Rückkehr** aus der Mitte einer Funktion (kein `return` an beliebiger Stelle möglich – jedes Statement wird immer bis zum Ende ausgeführt).

### 6.6 Operatoren – genaue Semantik

**Arithmetik (`+ - * / %`)** – nur für `Number op Number` definiert:
- `+`, `-`, `*`: Standard.
- `/`: **Division durch 0 liefert `0.0`** statt Fehler/`Infinity`/`NaN`.
- `%`: ⚠️ **Bug – Operandenreihenfolge vertauscht!** Implementiert als `toInt(rhs) % toInt(lhs)`, also **`a % b` berechnet tatsächlich `b % a`**. Beispiel: `5 % 2` ergibt im Referenzinterpreter `2 % 5 = 2`, nicht die erwarteten `1`. **Für Hover/Diagnostics unbedingt dokumentieren, da dies vom mathematisch erwarteten Verhalten abweicht.**
- `toInt()` (für `%`) truncatet einfach via impliziter `float→int`-Konvertierung (Richtung Null).
- ⚠️ **Kein `+` für Strings (keine Konkatenation).** `"a" + "b"` fällt durch alle Prüfungen (nur `Number`-Typen werden für `evaluate_numeric_binary_expr` akzeptiert) und ergibt `Null`.
- Arithmetik zwischen inkompatiblen Typen (z. B. `Number + String`, `Object + Object`) ergibt still `Null` (kein Fehler).

**Vergleich (`== != < > <= >=`):**
- `==`/`!=` sind **typsicher**: unterschiedliche `RuntimeVal::Type` → sofort `false`/`true`. Innerhalb gleichen Typs: `Null==Null` immer `true`; `Number`/`String`/`Boolean` werden wertbasiert verglichen. **`Object`, `Array`, `FnValue` haben keine definierte Gleichheit** (fallen in den `default`-Zweig → `==` liefert immer `false`, `!=` immer `true`, unabhängig vom Inhalt).
- `< > <= >=` arbeiten **ungeprüft direkt auf `.number`** – werden auch auf Nicht-Zahlen angewendet (z. B. Strings), was implizit `0 < 0` (da `.number` bei Strings `0` ist) statt eines Fehlers ergibt. **Kein Typ-Check bei relationalen Operatoren.**

**Logische Operatoren (`&& ||`):**
- **Kein Short-Circuiting** – `lhs` und `rhs` werden **immer beide ausgewertet** (`evaluate(*binop.left, env)` und `evaluate(*binop.right, env)` laufen vor der Operator-Prüfung). Nebenwirkungen auf der rechten Seite treten also **immer** ein, selbst wenn `&&` durch die linke Seite bereits `false` feststeht. Für ein LSP relevant bei Warnungen zu Seiteneffekten in `&&`/`||`.
- Ergebnis basiert auf `isTruthy()` beider Seiten (siehe 6.8), Ergebnistyp ist immer `Boolean`.

**Unäre Operatoren (`! -`):**
- `-x`: nur für `Number` definiert, sonst `Null`.
- `!x`: für **jeden** Typ definiert über `isTruthy()` (logische Negation), Ergebnis immer `Boolean`.
- Kein unäres `+`.

### 6.7 Truthiness (`isTruthy()`)
| Typ | Truthy wenn |
|---|---|
| `Null` | nie (immer `false`) |
| `Boolean` | eigener Wert |
| `Number` | `!= 0` |
| `String` | nicht leer |
| `Array` | nicht leer |
| `Object` | hat mindestens eine Property |
| `FnValue` / `Native_FnValue` | immer `true` (Default-Zweig) |

### 6.8 Member-/Index-Zugriff (`.` und `[...]`) – inkl. Bugs

| Basistyp | `.identifier` | `[computed]` |
|---|---|---|
| **Array** | nur `.length` unterstützt, alles andere → `Null` | Index muss `Number` sein, Out-of-Range → `Null` (kein Fehler/Exception) |
| **String** | nur `.length` unterstützt | ⚠️ **Nicht implementiert / undefiniertes Verhalten:** der String-Zweig behandelt **nur** den nicht-computed Fall; bei `str[0]` (computed) wird `property` unbedingt auf `Identifier*` gecastet, obwohl es tatsächlich ein `NumericLiteral` ist → **Undefined Behavior** im Referenz-Interpreter. **Für das LSP: `string[index]` als nicht unterstützt/fehlerhaft markieren.** |
| **Object** | Property-Lookup über Klarnamen, fehlender Key → `Null` | ⚠️ **Bug:** Der berechnete Schlüssel wird über `RuntimeVal::toString()` gebildet, was **immer** die volle Debug-Repräsentation liefert (z. B. `{ type: "string", value: "key" }` statt `"key"`). Dadurch funktioniert `obj["key"]` in der Praxis **nicht** wie erwartet (kein Treffer im internen Map, da die Property beim Literal `{key: ...}` unter dem Klarnamen `"key"` abgelegt wurde). **Computed Object-Access ist im Referenzinterpreter faktisch defekt.** |
| **Null / andere** | → `Null` | → `Null` |

---

## 7. Standardbibliothek / eingebaute Werte

Definiert in `runtime/envirments.cpp::setupStandardEnvironment` (nur im **globalen** Root-Environment, da `Environment`-Konstruktor `setupStandardEnvironment` nur aufruft, wenn kein `parent` übergeben wurde):

| Name | Typ | Wert / Verhalten |
|---|---|---|
| `true` | `const Boolean` | `true` |
| `false` | `const Boolean` | `false` |
| `null` | `const Null` | `Null` |
| `print` | `const Native_FnValue` | Variadische Funktion, gibt alle Argumente space-separiert + Zeilenumbruch auf `stdout` aus. **Arrays können nicht geprintet werden** (`print([1,2])` gibt literal den Text `"Can not print <array>"` statt der Werte aus – kein Crash, aber keine sinnvolle Ausgabe). `Object`-Werte werden nur als Platzhalter `<object>` ausgegeben (keine Property-Details). |

Das ist die **gesamte** Standardbibliothek – kein `len()`, `push()`, `Math.*`, String-Methoden, I/O (außer `print`), Typumwandlungsfunktionen etc.

---

## 8. Fehlerbehandlungsmodell

Zwei fundamental unterschiedliche Fehlerklassen – wichtig für die Unterscheidung zwischen **LSP-Diagnostics** (statisch, vor Ausführung) und dem **tatsächlichen Laufzeitverhalten**:

1. **Parser-Fehler (i. d. R. fatal):** `Parser::expect()` gibt bei falschem Token eine ausführliche Fehlermeldung auf `stderr` aus (Kontext, erwarteter/gefundener Tokentyp, Zeile/Spalte) und ruft **`std::exit(1)`** auf – das Programm terminiert sofort, keine Wiederherstellung. Betrifft: fehlende `;`, fehlende `{`/`}`/`)`/`]`, falsches Token nach `let`/`const`, etc.
2. **Teilweise wiederherstellbare Parser-Fehler:** Einige Stellen (`parse_primary_expr`-`default`-Fall bei unerwartetem Token, `parse_fn_declaration` bei ungültigen Parameternamen) geben eine Meldung aus, überspringen das fehlerhafte Token/liefern `nullptr` zurück und **versuchen weiterzuparsen** (kann zu Folgefehlern/Kaskaden führen).
3. **Laufzeit-"Fehler" (nicht fatal):** Undeklarierte Variablen, Redeklaration, Zuweisung an `const`, nicht aufrufbare Werte (`assert(false)` bei Aufruf eines Nicht-Funktionswerts – das ist der **einzige** Punkt, an dem zur Laufzeit ein harter Absturz via `assert` droht, sofern nicht im Release-Build `NDEBUG` gesetzt ist) – ansonsten werden Fehler nur über `stdout`/`stderr` gemeldet und der Interpreter läuft mit `Null`-Ersatzwerten weiter.

**Konsequenz für den LSP:** Da der Referenzinterpreter viele Fehler nur "leise" toleriert (still `Null` zurückgibt), muss die statische Analyse des LSP **eigenständig** und **strenger** prüfen (undeklarierte Variablen, `const`-Reassignment, Typ-Inkompatibilitäten, nicht unterstützte Zuweisungsziele etc.), da sich der Nutzer nicht auf Laufzeit-Exceptions verlassen kann.

---

## 9. Bekannte Bugs & Kuriositäten (Kompaktliste für Diagnostics/Hover)

| # | Bug/Verhalten | Auswirkung |
|---|---|---|
| B1 | `%`-Operator vertauscht Operanden (`a % b` = `b % a`) | Falsche Ergebnisse bei Modulo |
| B2 | Keine Fließkomma-Literale im Lexer (`3.14` bricht Parsing ab) | Praktisch keine Dezimalzahlen im Quelltext möglich |
| B3 | `+` konkateniert keine Strings | `"a"+"b"` → `Null` |
| B4 | Computed Object-Access `obj[key]` faktisch defekt (Debug-String als Key) | Liefert praktisch immer `Null` |
| B5 | `string[index]` verursacht UB (falscher Cast) | Nicht verwenden |
| B6 | Zuweisung an `MemberExpr` (`obj.x = ...`, `arr[0] = ...`) syntaktisch erlaubt, aber UB zur Laufzeit | Sollte vom LSP als Fehler markiert werden |
| B7 | Kein Short-Circuit bei `&&`/`||` | Seiteneffekte auf beiden Seiten immer ausgeführt |
| B8 | `if`/`else`-Blöcke erzeugen kein eigenes Scope (nur `while`/Funktionsaufrufe tun das) | `let` in `if`-Block "leakt" ins umgebende Scope-Verhalten |
| B9 | Identifier ohne `_` erlaubt | Restriktiver als die meisten Sprachen |
| B10 | Keine String-Escapes | `\"` im String nicht möglich |
| B11 | `true`/`false`/`null` sind keine Keywords, sondern schattierbare Konstanten | `let true = 5;` in einem inneren Scope ist gültig |
| B12 | Division durch 0 → `0` statt Fehler/Infinity | Stille Fehlkalkulation |
| B13 | Kein `.`/`[...]`-Zugriff nach einem Funktionsaufruf (`foo().bar` ungültig) | Aufruf-Verkettung eingeschränkt |
| B14 | Semikolon nach Ausdrucks-Statements führt zu Parser-Fehler/verlorenem Statement | Nutzer sollten hier auf `;` verzichten |
| B15 | `evaluate(program, *env)` in `ffi.cpp` übergibt `Environment&` statt `shared_ptr<Environment>` an `evaluate()` | Typinkonsistenz im FFI-Glue-Code (Build-Risiko in der Referenzimplementierung) |
| B16 | Objekt-Property-Iteration unsortiert (`unordered_map`) | Keine garantierte Reihenfolge bei Objekt-Ausgabe/-Iteration |
| B17 | Keine Arity-Prüfung bei Funktionsaufrufen | Zu wenige/zu viele Argumente werden stillschweigend akzeptiert |

---

## 10. Fehlende Sprachfeatures (Nicht-Ziel-Liste / Roadmap-Kandidaten)

Für die Abgrenzung dessen, was ein LSP **nicht** unterstützen muss (weil die Sprache es nicht kennt):

- Kein `return`, `break`, `continue`
- Kein `for`, `for-in`, `for-of`, `switch`/`match`
- Keine Lambdas/anonyme Funktionen, keine Closures-Syntax außer benannten `fn`
- Kein Modul-/Importsystem (`import`, `require`, `use`)
- Keine Klassen/Structs/Vererbung, keine Methoden auf Objekten
- Keine Typannotationen/statische Typisierung, keine Generics
- Keine Fehlerbehandlung (`try`/`catch`/`throw`)
- Kein Spread-/Rest-Operator, keine Destrukturierung
- Kein Bitweise-Operatoren (`&`, `|`, `^`, `<<`, `>>` – Lexer erkennt einzelnes `&`/`|` gar nicht)
- Keine Template-/interpolierten Strings
- Keine ternären Operatoren (`?:`)
- Kein `Optional Chaining` (`?.`)
- Kein `null`-Koaleszenz-Operator (`??`)
- Keine Mehrfachzuweisung/Tupel
- Kein Blockausdruck als Statement (`{ ... }` allein wird als Objekt-Literal geparst)

---

## 11. Vorschlag: Feature-Mapping für den LSP

Basierend auf obiger Analyse – konkrete Empfehlungen, was der LSP leisten sollte:

### 11.1 Diagnostics (statische Analyse, da Interpreter selbst kaum Fehler wirft)
- Syntaxfehler (Parser-1:1-Nachbau: erwartete vs. gefundene Tokens, mit Zeile/Spalte)
- Undeklarierte Variablen/Funktionen (`env`-Lookup-Simulation über Scope-Kette)
- Redeklaration im selben Scope (B: „Can not redeclare")
- Zuweisung an `const`-Variable
- Zuweisung an Nicht-Identifier-Ziel (B6) → Fehler, da UB im Interpreter
- Verwendung von `_` in Identifiern → Syntaxfehler (Lexer-Regel)
- Dezimalpunkt-Literale (`3.14`) → Fehler/Warnung (B2)
- String-Konkatenation mit `+` → Warnung „wird zu `Null` ausgewertet" (B3)
- `%`-Nutzung → Hinweis auf vertauschte Operandenreihenfolge (B1), evtl. nur als Hover statt Diagnostic
- Computed Object-Access `obj[...]` → Warnung „funktioniert im Referenzinterpreter nicht zuverlässig" (B4)
- `string[index]` → Fehler (B5)
- Falsche Anzahl Funktionsargumente → optionale Warnung (Sprache selbst prüft nicht, aber sinnvoll für Nutzer)
- Fehlendes `;` nach `let`/`const`
- Überflüssiges `;` nach Ausdrucks-Statement (B14)
- Aufrufverkettung `foo().bar` / `foo()[0]` → Syntaxfehler (B13)
- `else if` als Kette (nicht existent) → ggf. Hinweistext mit Verweis auf `else { if ... }`

### 11.2 Hover
- Für Operatoren `%`, `/`, `+`: tatsächliches Laufzeitverhalten anzeigen (inkl. B1/B12/B3-Hinweise)
- Für Identifier: aufgelöster Typ (best-effort, da dynamisch typisiert) + Deklarationsort + `const`/`let`
- Für Funktionen: Parameterliste, Hinweis auf implizite Return-Regel (letztes Statement oder `result`-Variable)

### 11.3 Go to Definition / Find References
- Über Scope-Kette (Environment-Modell nachbilden: Funktionskörper, `while`-Bodies als eigene Scopes; `if`/`else` **nicht** als eigene Scopes, siehe B8)

### 11.4 Completion
- Keywords: `let const fn if else while`
- Globale Vordeklarationen: `true false null print`
- Kontextabhängig: lokale Variablen/Parameter im aktuellen Scope, Objekt-Properties nach `.` (nur bekannte/inferierte Keys, da `obj["x"]` nicht zuverlässig funktioniert), `.length` bei Array-/String-Ausdrücken

### 11.5 Semantic Tokens / Syntax Highlighting
- Kategorien: Keyword, Identifier, Number, String, Operator, Comment (`#`/`//`), Punctuation (`{}[]()` `,` `.` `:` `;`), Function-Declaration-Name, Parameter

### 11.6 Formatierung
- Kanonische Regeln ableitbar: 2-Leerzeichen-Einrückung (siehe `indentStr` im Referenzcode als Konvention), `{` am Zeilenende (K&R-Stil), Leerzeichen um Binäroperatoren, `;` ausschließlich nach `let`/`const`.

---

## 12. Quelldatei-Übersicht (Mapping der Referenzimplementierung)

| Datei | Inhalt |
|---|---|
| `frontend/lexer.hpp/.cpp` | Tokenizer, `TokenType`-Enum, Keyword-Tabelle |
| `frontend/ast.hpp/.cpp` | AST-Knotendefinitionen, `NodeType`-Enum, `toString()`/`clone()` |
| `frontend/parser.hpp/.cpp` | Rekursiver Abstiegsparser, Grammatik/Präzedenz |
| `runtime/values.hpp` | `RuntimeVal` – dynamischer Werttyp (tagged union über `Type`-Enum) |
| `runtime/envirments.hpp/.cpp` | `Environment`-Klasse (Scope-Kette), Standardbibliothek-Setup |
| `runtime/interpreter.hpp/.cpp` | zentrale `evaluate()`-Dispatch-Funktion (Switch über `NodeType`) |
| `runtime/eval/expressions.cpp/.hpp` | Auswertung: Binary/Unary/Assignment/Object/Call-Expressions |
| `runtime/eval/statements.cpp/.hpp` | Auswertung: Program, VarDeclaration, FunctionDeclaration |
| `rustffi/ffi.hpp/.cpp` | Rust-Bridge (`cxx`), `run_file()`, `runREPL()` |
| `util.hpp/.cpp` | Hilfsfunktionen (`toInt`), `ERROR`-Makro |

---

## 13. Beispielprogramm (gültige Fling-Syntax, aus der Grammatik abgeleitet)

```fling
# Kommentar mit #
// Kommentar mit //

let x = 10;
const y = 20;

fn add(a, b) {
    a + b
}

if x < y {
    print("x ist kleiner als y", x, y);
} else {
    print("x ist groesser oder gleich y");
}

let i = 0;
while i < 3 {
    print("i =", i);
    i = i + 1;
}

let liste = [1, 2, 3,];
print(liste.length);

let obj = { name: "Fling", version: 1 };
print(obj.name);

let ergebnis = add(x, y);
print(ergebnis);
```

**Nicht gültig (zur Abgrenzung):**
```fling
let z = 3.14;        // FEHLER: keine Dezimalliterale im Lexer
let my_var = 5;       // FEHLER: '_' nicht erlaubt in Identifiern
print("hi");;         // zweites ';' ist ein Fehler (kein Semikolon nach Ausdruck erlaubt)
obj["name"] = "x";     // Parsebar, aber Zuweisung an MemberExpr ist UB
foo().bar;             // FEHLER: Member-Zugriff nach Call nicht unterstützt
if x > 0 { } else if y > 0 { }   // FEHLER: 'else if' existiert nicht, nur 'else { if ... }'
```

---

## 14. Zusammenfassung für die LSP-Roadmap

1. **Phase 1 – Tokenizer/Parser 1:1 nachbauen** (idealerweise in Rust, passend zur bestehenden FFI-Anbindung) inkl. exakter Fehlerpositionen.
2. **Phase 2 – Statische Scope-/Symbol-Analyse** (Environment-Modell nachbilden, inkl. der Besonderheit, dass `if`/`else` kein eigenes Scope hat).
3. **Phase 3 – Diagnostics-Regelwerk** gemäß Abschnitt 11.1, priorisiert nach den in Abschnitt 9 gelisteten Bugs (B1–B17), da diese die häufigsten Quellen für „stilles Fehlverhalten" sind, das ein Nutzer ohne LSP nur schwer bemerkt.
4. **Phase 4 – Hover/Completion/Go-to-Definition** auf Basis des Symbol-Modells.
5. **Phase 5 – Optional:** Formatter/Codeaktionen (z. B. automatischer Fix für `else if`-Ketten → `else { if ... }`-Umschreibung, Entfernen überflüssiger Semikola).

