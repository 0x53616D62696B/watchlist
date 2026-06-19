# C++ Anti-Patterns

C++ gives direct control over memory, lifetime, value semantics, and performance.
That power also makes certain design mistakes especially expensive.

Many C++ anti-patterns come from fighting the language instead of using its strengths: RAII, value types, explicit ownership, and standard library abstractions.

## Manual Memory Ownership With Raw Pointers

Owning memory through raw pointers is one of the most common C++ anti-patterns.

Example:

```cpp
Widget* widget = new Widget();
UseWidget(widget);
delete widget;
```

Why it is harmful:

- Early returns and exceptions can leak memory.
- Ownership is unclear to readers.
- Double deletion and dangling pointers become possible.

Better approach:

- Prefer automatic storage duration when possible.
- Use `std::unique_ptr` for exclusive ownership.
- Use `std::shared_ptr` only when shared lifetime is genuinely required.

```cpp
auto widget = std::make_unique<Widget>();
UseWidget(*widget);
```

## Calling `new` And `delete` In Application Logic

Even when leaks do not happen, explicit heap management scattered through business logic is usually a design smell.

Why it is harmful:

- Lifetime rules are spread across the codebase.
- Ownership becomes a convention instead of a property of the type.
- Exception safety becomes harder.

Better approach:

- Wrap resource ownership in RAII types.
- Construct objects directly in containers or owning wrappers.

## Non-RAII Resource Management

Memory is not the only resource.
Files, sockets, mutexes, threads, and database handles also need correct cleanup.

Anti-pattern shape:

```cpp
Lock(mutex);
DoWork();
Unlock(mutex);
```

Why it is harmful:

- Cleanup is easy to skip on error paths.
- The code is not exception-safe.
- Human discipline is treated as the cleanup mechanism.

Better approach:

- Use destructors for cleanup.
- Prefer standard wrappers such as `std::lock_guard`, `std::unique_lock`, `std::ofstream`, and smart pointers.

## Using `std::shared_ptr` By Default

`std::shared_ptr` is useful, but using it as the default ownership tool is an anti-pattern.

Why it is harmful:

- Shared ownership hides lifetime decisions.
- Reference cycles can leak memory.
- Atomic reference counting can add unnecessary runtime cost.
- Code becomes less clear about who really owns an object.

Better approach:

- Start with stack objects or `std::unique_ptr`.
- Introduce `std::shared_ptr` only when several owners must extend lifetime independently.

## Returning Or Storing References To Short-Lived Objects

Example:

```cpp
const std::string& GetName() {
    std::string name = "watchlist";
    return name;
}
```

Why it is harmful:

- It creates dangling references.
- The program can appear to work and then fail unpredictably.

Better approach:

- Return by value when the function creates a value.
- Return references only when the referred object clearly outlives the caller's use.

## Inheritance Instead Of Value Composition

Some C++ designs use inheritance where plain members or policy objects would be simpler.

Why it is harmful:

- It complicates construction and destruction order.
- It increases coupling between types.
- It invites virtual dispatch where none is needed.

Better approach:

- Prefer composition for code reuse.
- Use templates, strategy objects, or free functions when behavior variation does not require subtype polymorphism.

## Virtual Functions Without Virtual Destructors

If a base class is meant to be deleted through a base pointer, it needs a virtual destructor.

Bad example:

```cpp
class Base {
public:
    virtual void Run() = 0;
};
```

Why it is harmful:

- Deleting a derived object through `Base*` is undefined behavior.

Better approach:

```cpp
class Base {
public:
    virtual ~Base() = default;
    virtual void Run() = 0;
};
```

## Macros For What The Language Already Solves

Heavy macro usage for constants, functions, or pseudo-generic code is usually a C++ anti-pattern.

Why it is harmful:

- Macros ignore scope and type rules.
- Errors are harder to debug.
- Readers must mentally expand preprocessor behavior.

Better approach:

- Prefer `constexpr`, `inline`, templates, enums, and ordinary functions.

## Using Exceptions Inconsistently

One harmful pattern is mixing exception-based and manual error conventions without a clear policy.

Example symptoms:

- Some functions throw, others return error codes, and some do both.
- Callers do not know whether cleanup is required before propagating failure.
- Error handling logic is duplicated at many layers.

Better approach:

- Choose a consistent project-level error-handling model.
- Make ownership and cleanup independent of that choice by relying on RAII.

## Premature Move Obsession

Modern C++ encourages move semantics, but forcing `std::move` everywhere is an anti-pattern.

Why it is harmful:

- It can prevent copy elision in some situations.
- It can move from objects that are still expected to be used.
- It makes code noisier without measurable benefit.

Better approach:

- Let value semantics work by default.
- Use `std::move` when transferring ownership from a named object intentionally.

## Overusing Output Parameters

Example:

```cpp
void ParseConfig(const std::string& text, Config& outConfig, bool& outValid);
```

Why it is harmful:

- The function contract is harder to read.
- Callers must prepare mutable state before the call.
- Error handling and ownership become less explicit.

Better approach:

- Return values directly.
- Use small result objects when several values belong together.

## Reimplementing Standard Library Containers

Creating custom string, vector, or smart pointer types without a strong reason is usually wasteful.

Why it is harmful:

- Standard containers already encode correct ownership and semantics.
- Custom containers are easy to get wrong.
- Maintenance cost rises without product value.

Better approach:

- Use `std::vector`, `std::string`, `std::array`, `std::optional`, `std::variant`, and other standard tools unless a real requirement proves otherwise.

## Ignoring Const Correctness

When an API does not communicate which operations mutate state, readers lose useful guarantees.

Why it is harmful:

- Safe usage becomes harder to reason about.
- Interfaces become less expressive.
- Accidental mutation is easier.

Better approach:

- Mark read-only operations `const`.
- Pass read-only inputs as `const` references where appropriate.
- Use immutability to make intent visible.

## How To Review C++ Code For Anti-Patterns

Ask these questions:

- Who owns this object and how is that ownership expressed?
- Does cleanup happen automatically through scope?
- Is polymorphism truly needed here?
- Could a value type or standard container express this more clearly?
- Are lifetime rules obvious from the API?
- Is this optimization measured or only assumed?

## Summary

C++ anti-patterns often come from unclear ownership, manual cleanup, unnecessary inheritance, and avoiding the standard tools the language provides.

The safest defaults are RAII, value semantics, explicit ownership, and small interfaces that make lifetime and mutation rules obvious.

## Sources Used For This Page

This page was drafted from established C++ engineering knowledge and aligned to the style of nearby theory documentation in this repository.
No external web source was consulted while writing it.

In-repository references used for tone and adjacent concepts:

- `docs/Theory/CPP/RAII.md`
- `docs/Theory/clean_code_principles.md`

Related background concepts referenced from general engineering knowledge:

- RAII
- const correctness
- value semantics
- standard library ownership patterns
