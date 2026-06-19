# C++ Anti-Patterns

C++ is strongest when code makes ownership, lifetime, and mutation rules explicit.
Many C++ anti-patterns come from bypassing those strengths and reintroducing manual lifetime management or unclear object models.

This page focuses on recurring mistakes that conflict with RAII, smart-pointer ownership, virtual-dispatch rules, and const-correct APIs.

## Manual Ownership Through Raw Pointers

Cppreference defines `std::unique_ptr` as a smart pointer that owns and manages another object, disposing of it automatically when the owning pointer is destroyed or reset.
Using raw owning pointers instead of that ownership model is one of the most common C++ anti-patterns.

Bad shape:

```cpp
Widget* widget = new Widget();
UseWidget(widget);
delete widget;
```

Why it is harmful:

- early returns and exceptions can leak resources
- ownership is invisible in the type system
- double delete and dangling pointer bugs become possible

Better approach:

- prefer automatic storage duration where possible
- use `std::unique_ptr` for exclusive ownership
- use `std::make_unique` instead of direct `new` in most application code

## Using `std::shared_ptr` By Default

Cppreference describes `std::shared_ptr` as a shared-ownership smart pointer with a control block and reference counting.
That makes it powerful, but also heavier and easier to misuse than `std::unique_ptr`.

Anti-pattern:

- using `std::shared_ptr` everywhere even when there is one clear owner

Why it is harmful:

- lifetime decisions become vague
- reference cycles are possible
- the control block and reference counting add cost and complexity
- code stops communicating who is responsible for object lifetime

Better approach:

- begin with stack objects or `std::unique_ptr`
- introduce `std::shared_ptr` only when multiple owners must independently prolong lifetime

## Constructing Smart Ownership Around A Raw Pointer Twice

Cppreference explicitly warns that constructing a new `shared_ptr` from a raw pointer already owned by another `shared_ptr` is undefined behavior.

This is a classic ownership anti-pattern:

- a raw pointer is extracted with `get()`
- another owning smart pointer is created from it

Why it is harmful:

- two independent owners now believe they should destroy the same object
- the result is undefined behavior

Better approach:

- share ownership by copying the existing `shared_ptr`
- pass raw pointers or references only as non-owning access

## Non-RAII Resource Management

RAII is not optional style in C++; it is the normal way to make cleanup reliable.
When code manually opens, locks, allocates, and then later tries to remember to release, cleanup is tied to human discipline instead of scope.

Why it is harmful:

- error paths skip cleanup
- exceptions break lifetime assumptions
- ownership becomes scattered across the function

Better approach:

- use destructors and scope-bound wrappers
- prefer `std::lock_guard`, `std::unique_lock`, file stream types, smart pointers, and resource-owning wrappers

## Polymorphic Base Without A Virtual Destructor

Cppreference's `virtual` documentation is direct: if a base class destructor is not virtual, deleting a derived object through a base pointer is undefined behavior.

Bad example:

```cpp
class Base {
public:
    virtual void Run() = 0;
};
```

Why it is harmful:

- destruction through `Base*` or owning base-pointer wrappers becomes unsafe
- derived resources may not be released correctly

Better approach:

```cpp
class Base {
public:
    virtual ~Base() = default;
    virtual void Run() = 0;
};
```

## Inheritance For Reuse Instead Of A True Type Relationship

Cppreference explains that virtual dispatch exists to preserve overriding behavior when working through base references or pointers.
That power should model real polymorphism, not generic reuse.

Anti-pattern symptoms:

- inheritance used only to reuse implementation
- derived classes override methods just to disable or reject behavior
- base classes expose members irrelevant to some subclasses

Why it is harmful:

- substitutability breaks
- object lifetime and destruction rules become more delicate
- behavior is spread across override chains

Better approach:

- prefer composition when the relationship is not truly "is-a"
- keep inheritance for stable interfaces and genuine subtype behavior

## Ignoring Const Correctness

Cppreference's cv-qualifier documentation describes `const` as part of the type system and explains qualification conversions and mutation rules.
Ignoring const-correctness hides intent and weakens API contracts.

Why it is harmful:

- callers cannot tell whether an operation mutates state
- non-mutating behavior cannot be relied on or composed safely
- accidental mutation becomes easier

Better approach:

- mark read-only member functions `const`
- pass read-only inputs through const-qualified references where appropriate
- use `mutable` only for members that do not change externally visible logical state, such as mutexes or caches

## `std::move` Everywhere

Move semantics are useful when ownership is intentionally transferred.
They are not a decoration to apply to every variable.

Why it is harmful:

- moved-from objects may still be used by mistake
- code becomes noisy and harder to reason about
- value-return optimization and normal value semantics are obscured

Better approach:

- rely on normal value semantics by default
- use `std::move` when a named object is genuinely being transferred

## Reimplementing Standard Facilities Without A Requirement

Rewriting string containers, pointer wrappers, or simple scope guards without a hard requirement is usually wasted effort.

Why it is harmful:

- standard-library behavior is already well-known and battle-tested
- custom replacements are easy to get subtly wrong
- maintenance cost rises with little product value

Better approach:

- prefer standard types first
- add custom infrastructure only when a real constraint justifies it

## Practical Review Checklist

When reviewing C++ code, ask:

- who owns this object and where is that ownership encoded?
- does cleanup happen automatically through scope?
- is shared ownership genuinely required?
- is this base class meant to be deleted polymorphically?
- does the API clearly distinguish mutating and non-mutating operations?
- is inheritance expressing a real type relationship or only code reuse?

## Summary

C++ anti-patterns usually come from unclear ownership, manual cleanup, casual inheritance, and APIs that hide lifetime or mutation semantics.
The safest defaults are RAII, explicit ownership, standard-library tools, and interfaces that make constness and destruction rules obvious.

## Web References

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [cppreference, `std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr)
- [cppreference, `std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr)
- [cppreference, `virtual` function specifier](https://en.cppreference.com/w/cpp/language/virtual)
- [cppreference, cv qualifiers](https://en.cppreference.com/w/cpp/language/cv)
- [Standard C++ Foundation](https://isocpp.org/)
