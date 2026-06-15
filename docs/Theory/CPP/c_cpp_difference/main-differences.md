# Main Differences Between C and C++

## Big Picture

C is a small procedural systems language. It gives direct control over memory, object representation, linkage, and calling conventions. Most abstractions are built manually with structs, functions, macros, function pointers, and conventions.

C++ started from C, but is a separate language with stronger type checking, deterministic object lifetime, templates, classes, references, overloads, exceptions, namespaces, and a large standard library. Modern C++ code often relies on RAII, containers, algorithms, smart pointers, and compile-time generic programming.

## Compilation Model

Both languages usually use preprocessing, compilation, and linking, but they are not interchangeable.

- C source is normally compiled as C with a C compiler mode.
- C++ source is compiled as C++ with a C++ compiler mode.
- Header files included by both C and C++ must avoid C++-only syntax unless guarded.
- C++ changes symbol names through name mangling, so C-linkable APIs need `extern "C"`.
- C++ treats some C constructs as errors or as different types.

## Type System

C++ has stricter and richer typing.

- C allows implicit conversion from `void*` to object pointers. C++ requires an explicit cast.
- C++ has references, overload resolution, `const` member functions, stronger enum options, classes, templates, and namespaces.
- C++ has `bool` as a built-in type. C uses `_Bool` and usually `<stdbool.h>`.
- Character literals differ: in C, `'a'` has type `int`; in C++, it has type `char`.
- Empty parameter lists differ: `int f();` means unspecified parameters in C, but no parameters in C++.

## Object Lifetime and Initialization

C primarily initializes storage. C++ initializes objects.

- C++ constructors and destructors run automatically.
- C++ stack objects can manage resources through RAII.
- C++ initialization syntax is more varied: direct, copy, list, aggregate, value, and default initialization.
- C++ object lifetime rules matter even for raw storage, placement new, unions, and casts.
- C uses explicit init and cleanup functions by convention.

## Memory Management

C normally uses explicit allocation and release.

- Common C tools: `malloc`, `calloc`, `realloc`, `free`.
- Common C++ tools: automatic storage, standard containers, `std::unique_ptr`, `std::shared_ptr`, `new`, `delete`, and custom allocators.
- Memory allocated with `malloc` must be released with `free`.
- Memory allocated with `new` must be released with `delete`.
- Memory allocated with `new[]` must be released with `delete[]`.
- Mixing allocation families is undefined behavior.

Modern C++ usually avoids owning raw pointers. In C, pointer ownership must be documented explicitly.

## Error Handling

C usually reports errors through return values, output parameters, global or thread-local error state, or `errno`.

C++ can use those techniques, but also supports exceptions and richer return types.

- C: `int`, `enum`, `NULL`, `errno`, sentinel values, result structs.
- C++: exceptions, `std::optional`, `std::expected` where available, `std::error_code`, RAII rollback, result types.

If code crosses a C ABI boundary, do not let C++ exceptions escape.

## Abstraction Mechanisms

C builds abstractions with:

- `struct`
- function prefixes
- opaque pointers
- function pointers
- macros
- manual vtables
- naming conventions

C++ additionally has:

- classes and access control
- constructors and destructors
- inheritance and virtual functions
- templates
- overloads
- namespaces
- lambdas
- operator overloads
- standard library containers and algorithms

## Standard Libraries

C has a smaller standard library focused on low-level utilities, memory, strings, files, math, time, and locale.

C++ includes most of the C library plus the C++ standard library:

- containers: `std::vector`, `std::array`, `std::map`, `std::unordered_map`
- strings: `std::string`, `std::string_view`
- algorithms: `std::sort`, `std::find`, `std::transform`
- memory: `std::unique_ptr`, `std::shared_ptr`
- I/O: streams and filesystem
- concurrency: threads, atomics, mutexes, condition variables
- utilities: tuples, variants, optionals, spans, ranges in newer standards

## Preprocessor Use

C relies more heavily on macros because it lacks templates, overloads, namespaces, and constants with the same expressive power as C++.

In C++, prefer:

- `constexpr` or `const` instead of object-like macros
- templates or inline functions instead of function-like macros
- namespaces instead of prefix-only naming
- enum classes instead of unscoped integer constants when appropriate

Macros still matter in both languages for conditional compilation, include guards, attributes, and platform glue.

## ABI and Interoperability

C has a simpler and more stable ABI story across compilers and languages. Many languages can call C APIs directly.

C++ ABI is more complex because of:

- name mangling
- exceptions
- RTTI
- class layout
- virtual tables
- templates
- standard library ABI differences

When building a library boundary, a C API can be a good stable interface even if the implementation is C++.

## Style Difference

Idiomatic C:

- explicit ownership
- simple data structures
- small functions
- return-code error handling
- opaque handles for encapsulation
- minimal hidden work

Idiomatic C++:

- RAII ownership
- value types
- standard containers
- smart pointers only where ownership is dynamic
- algorithms over raw loops where clear
- exceptions or typed results depending on project style
- strong types and scoped names

The most important practical difference: C makes lifetimes visible through conventions; C++ can encode lifetimes into types.
