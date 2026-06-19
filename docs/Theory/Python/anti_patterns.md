# Python Anti-Patterns

Python supports several styles: object-oriented, procedural, functional, and scripting-oriented code.
Many Python anti-patterns appear when code ignores Python's strengths or when dynamic behavior is used without discipline.

This page focuses on recurring mistakes that reduce readability, correctness, or maintainability in Python code.

## God Module Or God Class

In Python, large responsibilities often accumulate at the module level as well as inside classes.

Common symptoms:

- One file holds configuration, I/O, parsing, business rules, and command-line behavior.
- One class owns state changes, network calls, serialization, and validation.
- Imports from one module pull in many unrelated dependencies.

Why it is harmful:

- Testing becomes harder because setup drags in unrelated concerns.
- Import-time side effects become more likely.
- Small changes cause broad regressions.

Better approach:

- Split code by responsibility.
- Keep modules focused.
- Separate pure logic from side effects such as file I/O, HTTP, and environment access.

## Mutable Default Arguments

This is one of the best-known Python anti-patterns.

Bad example:

```python
def add_symbol(symbol: str, symbols: list[str] = []):
    symbols.append(symbol)
    return symbols
```

The default list is created once, not once per call.

Why it is harmful:

- State leaks across calls.
- Bugs can stay hidden until the function is reused in a different context.

Better approach:

```python
def add_symbol(symbol: str, symbols: list[str] | None = None) -> list[str]:
    if symbols is None:
        symbols = []
    symbols.append(symbol)
    return symbols
```

## Catching Broad Exceptions Without A Clear Policy

Example:

```python
try:
    process_data()
except Exception:
    return None
```

Why it is harmful:

- Real defects can be silently hidden.
- Debugging becomes much harder.
- Callers lose information about what failed.

Better approach:

- Catch only the exceptions you can handle meaningfully.
- Log or re-raise when the failure is not local and recoverable.

## Using Classes For Namespacing Only

Sometimes Python code creates classes only to group functions that share no instance state.

Example shape:

```python
class StringUtils:
    @staticmethod
    def normalize(text: str) -> str:
        return text.strip().lower()
```

Why it is harmful:

- It adds object-oriented ceremony without object-oriented value.
- The class suggests state or subtype relationships that do not exist.

Better approach:

- Use plain functions in a focused module when behavior does not depend on object state.

## Hidden Import-Time Side Effects

Python executes module top-level code at import time.
That makes import-time side effects especially risky.

Examples:

- Opening files or network connections during import.
- Reading environment state and mutating globals.
- Registering production behavior automatically when a module is imported.

Why it is harmful:

- Tests become order-dependent.
- Imports become slow and surprising.
- Simple tooling operations can trigger real side effects.

Better approach:

- Keep imports declarative.
- Move runtime actions into functions, factories, or explicit application startup code.

## Overusing Global State

Python makes global variables easy to create.
That convenience often turns into hidden coupling.

Why it is harmful:

- Functions depend on ambient state instead of explicit inputs.
- Tests interfere with each other.
- Concurrency and reuse become harder.

Better approach:

- Pass dependencies explicitly.
- Use configuration objects or dependency injection where helpful.
- Keep mutable state local when possible.

## Long Functions With Mixed Levels Of Abstraction

Python's syntax makes it easy to write long functions quickly.
That often leads to functions that mix parsing, validation, branching, persistence, and formatting in one place.

Why it is harmful:

- Readers cannot see the main flow clearly.
- Reuse and testing become harder.
- Small fixes risk breaking unrelated logic.

Better approach:

- Extract small functions around meaningful steps.
- Keep one level of abstraction per function where practical.

## Duck Typing Without Clear Protocols

Duck typing is powerful, but using it carelessly can make APIs vague.

Example symptoms:

- A function assumes an argument has several methods but documents none of them.
- Failures appear far from the call site as `AttributeError` or `TypeError`.

Better approach:

- Document expected behavior clearly.
- Use type hints and `Protocol` when they improve clarity.
- Validate inputs at meaningful boundaries.

## Misusing Inheritance Instead Of Composition

Python makes inheritance easy, but deep or convenience-based inheritance is still a design smell.

Why it is harmful:

- Behavior becomes harder to trace through `super()` chains.
- Multiple inheritance can become fragile when classes were not designed to cooperate.
- State invariants spread across several levels.

Better approach:

- Prefer composition when sharing behavior.
- Keep inheritance for genuine subtype relationships.
- Use mixins carefully and keep them small.

## Overusing Magic Methods And Clever Metaprogramming

Python supports descriptors, metaclasses, decorators, dynamic attribute access, and runtime patching.
These features are useful, but overusing them is an anti-pattern.

Why it is harmful:

- Code becomes hard to discover and debug.
- Tooling and type checking become less effective.
- New contributors must understand framework-like behavior before they can make small changes.

Better approach:

- Prefer explicit code unless metaprogramming removes substantial duplication or enables a clearly valuable framework mechanism.

## Boolean Flag APIs That Change Behavior Drastically

Example:

```python
def export_report(data: dict, detailed: bool, include_history: bool, silent: bool) -> str:
    ...
```

Why it is harmful:

- Call sites are hard to read.
- One function quietly handles many modes and responsibilities.
- Future changes tend to add more flags instead of better abstractions.

Better approach:

- Split distinct behaviors into separate functions.
- Use small configuration objects when options genuinely belong together.

## Ignoring Context Managers

Python provides `with` for reliable cleanup.
Avoiding it for files, locks, and similar resources is an anti-pattern.

Bad example:

```python
file = open("watchlist.txt", "w")
file.write("AAPL\n")
file.close()
```

Better approach:

```python
with open("watchlist.txt", "w", encoding="utf-8") as file:
    file.write("AAPL\n")
```

Why it is better:

- Cleanup is reliable.
- The resource lifetime is obvious from indentation and scope.

## How To Review Python Code For Anti-Patterns

Ask these questions:

- Does this module do one coherent job?
- Are side effects explicit or hidden at import time?
- Is mutable state local and intentional?
- Would a plain function be clearer than a class?
- Is dynamic behavior helping clarity or only adding surprise?
- Are cleanup and error-handling rules obvious?

## Summary

Python anti-patterns usually come from hidden state, hidden side effects, overuse of clever dynamic features, or applying object-oriented structure where simpler code would be clearer.

Good Python code tends to be explicit, focused, small in scope, and disciplined about resource management, exceptions, and module boundaries.

## Sources Used For This Page

This page was drafted from established Python engineering knowledge and aligned to the style of nearby theory documentation in this repository.
No external web source was consulted while writing it.

In-repository references used for tone and adjacent concepts:

- `docs/Theory/clean_code_principles.md`

Related background concepts referenced from general engineering knowledge:

- Python context manager usage
- exception-handling practices
- module design and side-effect control
- composition over inheritance
