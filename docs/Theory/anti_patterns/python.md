# Python Anti-Patterns

Python supports object-oriented, procedural, and functional styles.
Good Python code is usually explicit, small in scope, and disciplined about resource handling and exceptions.

Many Python anti-patterns come from relying on hidden behavior: shared mutable defaults, import-time side effects, ambiguous exception handling, or object models that are more clever than clear.

## Mutable Default Arguments

The Python language reference explains that default parameter values are evaluated once, at function definition time, and then reused on later calls.
This is why mutable defaults are a classic anti-pattern.

Bad example:

```python
def add_symbol(symbol: str, symbols: list[str] = []):
    symbols.append(symbol)
    return symbols
```

Why it is harmful:

- state leaks across calls
- bugs appear only after repeated use
- the function behaves like it has hidden global state

Better approach:

```python
def add_symbol(symbol: str, symbols: list[str] | None = None) -> list[str]:
    if symbols is None:
        symbols = []
    symbols.append(symbol)
    return symbols
```

## Relying On Finalization Instead Of Explicit Cleanup

The Python data model explicitly warns not to depend on immediate finalization when objects become unreachable and recommends explicit closing of external resources.
The `with` statement exists to encapsulate `try`/`finally` cleanup.

Anti-patterns:

- opening files and closing them manually everywhere
- assuming garbage collection will close resources quickly enough
- putting critical cleanup into `__del__()`

Why it is harmful:

- cleanup timing is not guaranteed across implementations
- `__del__()` has tricky semantics and can interact badly with cycles and shutdown
- resource leaks become intermittent and hard to reproduce

Better approach:

- use `with` for files, locks, and similar resources
- prefer explicit cleanup to object finalization
- reserve `__del__()` for very rare cases where its constraints are understood

## Catching Broad Exceptions Without Intent

The Python tutorial recommends being as specific as possible about the exceptions you intend to handle and allowing unexpected exceptions to propagate.

Bad example:

```python
try:
    process_data()
except Exception:
    return None
```

Why it is harmful:

- real defects are silently hidden
- callers lose useful failure information
- debugging becomes much harder

Better approach:

- catch only exceptions you can handle meaningfully
- log and re-raise unexpected failures when appropriate
- use `else` with `try` blocks to narrow the protected region when that improves clarity

## Import-Time Side Effects

The Python data model describes modules as a core organizational unit created and executed by the import system.
That means top-level module code runs during import, which makes import-time side effects particularly risky.

Anti-patterns:

- opening files or sockets when the module is imported
- mutating global configuration during import
- performing production registration or startup logic as a side effect of importing helpers

Why it is harmful:

- tests become order-dependent
- imports become slow and surprising
- static analysis or simple tooling actions can trigger real work

Better approach:

- keep module top level declarative
- move side effects into startup functions, factories, or application entry points

## Classes Used Only As Namespaces

Python classes are powerful, but creating a class only to group unrelated stateless helpers usually adds ceremony without value.

Bad shape:

```python
class StringUtils:
    @staticmethod
    def normalize(text: str) -> str:
        return text.strip().lower()
```

Why it is harmful:

- it suggests an object model that does not actually exist
- it makes simple functions harder to reach and test

Better approach:

- use plain functions in a focused module unless instance or class state is part of the design

## Overusing Magic Methods, Descriptors, Or Metaclasses

The Python data model shows how extensive the customization hooks are: descriptors, `__getattr__`, `__getattribute__`, metaclasses, and many special methods can redefine behavior deeply.

These are useful tools, but overusing them is an anti-pattern.

Why it is harmful:

- control flow becomes implicit
- debugging and tooling become harder
- maintainers must understand framework-like internals before making small changes

Better approach:

- prefer explicit code first
- introduce advanced hooks only when they remove real duplication or implement an important framework boundary

## Misusing Inheritance And MRO Complexity

The Python data model describes method resolution order and multiple inheritance rules in detail.
That should be a warning sign: inheritance in Python is flexible, but not free.

Anti-pattern symptoms:

- deep inheritance chains with fragile `super()` expectations
- mixins that assume hidden state or ordering
- multiple inheritance used where composition would be simpler

Why it is harmful:

- behavior becomes hard to trace through MRO
- subclassing rules become brittle
- accidental coupling between bases increases

Better approach:

- prefer composition when sharing behavior
- keep mixins small and narrow in purpose
- use inheritance only for real subtype relationships

## Mutable Global State

Python's module system makes globals convenient, but convenience often turns into hidden coupling.

Why it is harmful:

- functions depend on ambient state instead of explicit inputs
- tests interfere with each other
- concurrency and reuse become harder

Better approach:

- pass dependencies explicitly
- group configuration in explicit objects where useful
- keep mutable state local when possible

## Practical Review Checklist

When reviewing Python code, ask:

- is this shared state explicit or hidden?
- is cleanup deterministic, or does it depend on garbage collection?
- are exceptions being handled precisely or swallowed broadly?
- does this class model real state and behavior, or only provide ceremony?
- is advanced dynamic behavior improving clarity or only adding surprise?
- would a smaller module or plain function be simpler?

## Summary

Python anti-patterns usually come from hidden mutability, hidden side effects, and object or metaprogramming structures that are more clever than necessary.
Good Python code tends to be explicit about resource cleanup, exceptions, state, and module boundaries.

## Web References

- [Python 3 documentation home](https://docs.python.org/3/)
- [Python language reference, data model](https://docs.python.org/3/reference/datamodel.html)
- [Python language reference, the `with` statement](https://docs.python.org/3/reference/compound_stmts.html#the-with-statement)
- [Python tutorial, errors and exceptions](https://docs.python.org/3/tutorial/errors.html)
- [Refactoring.Guru code smells catalog](https://refactoring.guru/refactoring/smells)
- [Martin Fowler, software architecture and refactoring articles](https://martinfowler.com/)
