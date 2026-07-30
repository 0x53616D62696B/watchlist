# Storage Classes in C++

Storage class specifiers describe important properties of a declaration: lifetime, linkage, thread ownership, and mutability in special cases.
They do not directly mean "stack" or "heap"; those are common implementation details.

In C++, it is useful to separate four related ideas:

| Idea | Meaning | Example Question |
| --- | --- | --- |
| Storage duration | How long the object exists | Does it die at scope exit or at program exit? |
| Scope | Where the name can be used | Can this name be seen inside this block or file? |
| Linkage | Whether the same name can refer to the same entity across translation units | Can another `.cpp` file refer to this name? |
| Storage class specifier | Keyword that changes declaration behavior | Is this `static`, `extern`, `thread_local`, or `mutable`? |

## Quick Reference

| Specifier | Common Use | Main Effect |
| --- | --- | --- |
| `static` on a local variable | Persistent function-local state | Static storage duration, initialized once |
| `static` at namespace scope | Internal linkage | Name is visible only in the current translation unit |
| `extern` | Declaration of something defined elsewhere | External linkage, no definition by itself unless initialized |
| `thread_local` | Per-thread variable | One instance per thread |
| `mutable` | Cache or bookkeeping inside a `const` object | Member can be modified through a `const` object |
| `auto` | Type deduction | Not a storage class specifier in modern C++ |
| `register` | Historical optimization hint | Obsolete; do not use in modern C++ |

## Storage Duration

Storage duration says how long an object exists.

| Storage Duration | Example | Lifetime |
| --- | --- | --- |
| Automatic | `int value = 1;` inside a function | Created when execution reaches the declaration, destroyed when scope exits |
| Static | global variable, namespace variable, local `static` | Exists for the whole program |
| Thread | `thread_local int value;` | Exists for the lifetime of one thread |
| Dynamic | `new int{1}` or `std::make_unique<int>(1)` | Exists until explicitly released or owner destroys it |

Example:

```cpp
#include <memory>

int global_count = 0; // static storage duration

void example() {
    int local_count = 0;                         // automatic storage duration
    static int persistent_count = 0;             // static storage duration
    auto heap_count = std::make_unique<int>(0);  // dynamic storage duration for the int
}
```

The pointer-like owner `heap_count` is a local automatic object.
The `int` it owns is dynamically allocated and is released when `heap_count` is destroyed.

## `static`

`static` has two common meanings depending on where it appears.

## Local `static`

A local `static` variable is created once and keeps its value between function calls.

```cpp
int next_id() {
    static int id = 0;
    return ++id;
}
```

`id` is visible only inside `next_id`, but its lifetime lasts until the program ends.
It is initialized the first time execution reaches the declaration.

Use local `static` for:

- lazy single-instance objects
- counters that must persist between calls
- function-local caches

Be careful with hidden state. A function with local `static` data may be harder to test because repeated calls are no longer independent.

## Namespace-Scope `static`

At namespace scope, `static` gives a name internal linkage.
That means the name is visible only inside the current translation unit, usually one `.cpp` file after preprocessing.

```cpp
static int file_only_counter = 0;

static void helper() {
    ++file_only_counter;
}
```

Another `.cpp` file cannot refer to `file_only_counter` or `helper` by declaring them with `extern`.

In modern C++, an unnamed namespace is often preferred for file-local helpers:

```cpp
namespace {

int file_only_counter = 0;

void helper() {
    ++file_only_counter;
}

}
```

Both patterns restrict names to the current translation unit.

## Static Data Members

A `static` data member belongs to the class itself, not to each object.

```cpp
class Connection {
public:
    static int active_count;
};

int Connection::active_count = 0;
```

Every `Connection` object shares the same `active_count`.
Since C++17, `inline` variables make header-defined static data members easier:

```cpp
class Connection {
public:
    inline static int active_count = 0;
};
```

Use static data members for data that is logically shared by all objects of a type.

## `extern`

`extern` declares that a variable or function exists and is defined somewhere else.

Header:

```cpp
// config.hpp
extern int max_connections;
```

Source file:

```cpp
// config.cpp
int max_connections = 100;
```

Another source file can include the header and use the same variable:

```cpp
#include "config.hpp"

void print_limit() {
    // uses the max_connections defined in config.cpp
}
```

Important rule: put declarations in headers, but put exactly one non-`inline` variable definition in a `.cpp` file.

This is a declaration:

```cpp
extern int value;
```

This is a definition:

```cpp
int value = 42;
```

This is also a definition because it has an initializer:

```cpp
extern int value = 42;
```

## `thread_local`

`thread_local` gives each thread its own instance of a variable.

```cpp
thread_local int request_count = 0;

void handle_request() {
    ++request_count;
}
```

If four threads call `handle_request`, each thread increments its own `request_count`.
The variable is not shared between threads.

`thread_local` can be combined with `static` or `extern`:

```cpp
void worker() {
    static thread_local int retries = 0;
    ++retries;
}
```

Use `thread_local` for:

- per-thread caches
- per-thread counters
- thread-specific context

Do not use it as a general replacement for synchronization.
If threads need to communicate through shared state, use `std::atomic`, mutexes, condition variables, or another synchronization tool.

## `mutable`

`mutable` applies to non-static data members.
It allows that member to be changed even when the object is `const`.

```cpp
#include <cstddef>
#include <string>
#include <utility>

class User {
public:
    explicit User(std::string name)
        : name_(std::move(name)) {}

    std::size_t name_length() const {
        if (!cached_length_valid_) {
            cached_length_ = name_.size();
            cached_length_valid_ = true;
        }

        return cached_length_;
    }

private:
    std::string name_;
    mutable bool cached_length_valid_ = false;
    mutable std::size_t cached_length_ = 0;
};
```

`name_length()` is logically `const`: it does not change the user's name.
It only updates cache fields used to answer future calls faster.

Use `mutable` sparingly for:

- caches
- lazy calculations
- debug counters
- mutexes used inside `const` member functions

Avoid using `mutable` to hide real state changes from callers.

## `auto`

In modern C++, `auto` means type deduction.
It is not used as a storage class specifier anymore.

```cpp
auto count = 42;          // int
auto name = std::string{"Ada"};
```

The storage duration still depends on where the variable is declared:

```cpp
auto global_value = 1; // static storage duration

void example() {
    auto local_value = 2; // automatic storage duration
}
```

Use `auto` when the initializer makes the type clear or when spelling the type is noisy:

```cpp
auto it = values.begin();
auto user = make_user();
```

Avoid `auto` when it hides important information from the reader.

## `register`

`register` was an old hint asking the compiler to keep a variable in a CPU register.
Modern compilers choose register allocation themselves, so this hint is obsolete.

Do not use `register` in modern C++.

```cpp
register int value = 0; // old style; avoid
```

## Linkage

Linkage controls whether declarations in different scopes or translation units can refer to the same entity.

| Linkage | Meaning | Example |
| --- | --- | --- |
| No linkage | Name cannot be referred to from another scope declaration | local variable |
| Internal linkage | Same translation unit only | namespace-scope `static` variable |
| External linkage | Can be referred to from other translation units | ordinary global variable, non-static function |

Example:

```cpp
int shared_value = 1;        // external linkage
static int file_value = 2;   // internal linkage

void function() {
    int local_value = 3;     // no linkage
}
```

Header variables are a common source of linker errors.
This header creates a separate definition in every `.cpp` file that includes it:

```cpp
// bad.hpp
int value = 42;
```

Prefer one of these:

```cpp
// declaration.hpp
extern int value;
```

```cpp
// definition.cpp
int value = 42;
```

Or, since C++17:

```cpp
// constants.hpp
inline constexpr int value = 42;
```

## `const`, `constexpr`, and Storage

`const` and `constexpr` are not storage class specifiers, but they often appear near storage-related code.

```cpp
const int runtime_limit = 10;
constexpr int compile_time_limit = 10;
```

`const` means the object cannot be modified through that name.
`constexpr` means the value can be used in compile-time contexts if its initializer is valid for constant evaluation.

At namespace scope, `const` variables have internal linkage by default in C++:

```cpp
const int file_local_limit = 10; // internal linkage by default
```

Use `extern` if the same `const` object must be shared across translation units:

```cpp
// limits.hpp
extern const int shared_limit;
```

```cpp
// limits.cpp
extern const int shared_limit = 10;
```

For constants in headers, modern C++ often uses:

```cpp
inline constexpr int max_items = 100;
```

## Common Mistakes

### Thinking `static` Always Means One Thing

`static` changes meaning by context:

- local `static`: persistent local object
- namespace-scope `static`: file-local name
- class `static` member: shared member belonging to the class

Read the declaration location before interpreting the keyword.

### Defining Globals in Headers

This is usually wrong:

```cpp
// settings.hpp
int timeout_ms = 1000;
```

Every source file that includes the header gets a definition.
Use `extern` plus one `.cpp` definition, or use `inline constexpr` for constants.

### Using `static` for Shared Mutable State Too Freely

Global and static mutable state can make behavior order-dependent and harder to test.
Prefer passing dependencies explicitly unless persistent shared state is truly part of the design.

### Confusing Object Lifetime With Pointer Lifetime

```cpp
int* make_value() {
    int value = 42;
    return &value; // wrong: value dies at function exit
}
```

The pointer is just an address.
It does not extend the lifetime of the local object.

Better:

```cpp
int make_value() {
    return 42;
}
```

Or, when dynamic lifetime is really needed:

```cpp
#include <memory>

std::unique_ptr<int> make_value() {
    return std::make_unique<int>(42);
}
```

## Practical Rules

- Use ordinary local variables for short-lived local data.
- Use local `static` only when the function really needs persistent state.
- Prefer unnamed namespaces over namespace-scope `static` for file-local helpers in modern C++.
- Use `extern` declarations in headers and one definition in a source file for shared globals.
- Use `inline constexpr` for header constants in C++17 and newer.
- Use `thread_local` only for truly per-thread state.
- Use `mutable` only for logical constness, such as caches or mutexes.
- Avoid `register`.
- Remember that storage class specifiers do not directly mean stack or heap.

## Best Mental Model

When reading a declaration, ask four questions:

1. How long does the object live?
2. Where can this name be used?
3. Can another translation unit refer to the same entity?
4. Is the object shared globally, per thread, per class, or per call?

Those answers matter more than memorizing the keyword names alone.

## References

- cppreference, "Storage class specifiers": https://en.cppreference.com/w/cpp/language/storage_duration
- cppreference, "cv (const and volatile) type qualifiers": https://en.cppreference.com/w/cpp/language/cv
- cppreference, "Placeholder type specifiers": https://en.cppreference.com/w/cpp/language/auto
- C++ working draft, "[basic.stc] Storage duration": https://eel.is/c++draft/basic.stc
- C++ working draft, "[dcl.stc] Storage class specifiers": https://eel.is/c++draft/dcl.stc
- C++ working draft, "[basic.link] Linkage": https://eel.is/c++draft/basic.link
