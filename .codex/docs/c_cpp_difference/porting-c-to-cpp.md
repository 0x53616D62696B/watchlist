# Porting C Code to C++

Use this checklist when compiling C code as C++ or redesigning C code into idiomatic C++.

## First Decide the Goal

There are two different tasks:

- Compile existing C code with a C++ compiler.
- Rewrite or wrap C code into idiomatic C++.

The first task is mostly compatibility work. The second task is design work.

## Build and File Setup

- Rename implementation files only when the build should compile them as C++: `.c` to `.cpp`, `.cc`, or `.cxx`.
- Keep headers intended for both languages compatible with both C and C++.
- Add `extern "C"` guards around C APIs included from C++:

```c
#ifdef __cplusplus
extern "C" {
#endif

/* C declarations */

#ifdef __cplusplus
}
#endif
```

- Check compiler standard flags, warning levels, and platform macros.
- Verify all third-party headers are valid in C++ mode.

## Syntax and Type Compatibility

Check for C constructs that fail or change meaning in C++.

- Add explicit casts for `malloc`, `calloc`, and `realloc` results if you still use them.
- Prefer replacing allocation with constructors, containers, or smart pointers when redesigning.
- Do not use C++ keywords as identifiers: `class`, `new`, `delete`, `template`, `typename`, `namespace`, `operator`, `private`, `public`, `protected`, `this`, `try`, `catch`, `throw`, `bool`, `true`, `false`.
- Replace old-style function declarations like `int f();` with `int f(void);` in C-compatible headers or `int f();` in C++-only code when it truly takes no parameters.
- Check character literal assumptions because `'x'` is `int` in C and `char` in C++.
- Check enum conversions. C++ is stricter about converting integers to enum types.
- Check implicit pointer conversions, especially from `void*`.
- Check designated initializers. C++ supports a narrower form in C++20 and later.
- Check compound literals. They are C, not standard C++.
- Check variable length arrays. They are C99, not standard C++.
- Check flexible array members. They are C99, not standard C++.
- Check use of `restrict`. C++ does not have standard `restrict`, though compilers may support extensions.

## Headers and Linkage

- Wrap C declarations in `extern "C"` only when the implementation is compiled as C or exposes a C ABI.
- Do not put C++ templates, overloads, classes, references, or exceptions inside an `extern "C"` block.
- If a header must work in both languages, guard C++-only declarations with `#ifdef __cplusplus`.
- Avoid defining non-`static` objects or non-`inline` functions in headers.
- Use include guards or `#pragma once` consistently with the project style.

## Memory and Ownership

For a minimal port:

- Keep existing `malloc` and `free` pairs intact.
- Do not free memory allocated with `new`.
- Do not delete memory allocated with `malloc`.
- Check every ownership transfer and cleanup path.

For an idiomatic C++ redesign:

- Use automatic objects where possible.
- Use `std::vector<T>` for dynamic arrays.
- Use `std::array<T, N>` for fixed-size arrays.
- Use `std::string` or `std::string_view` for text where ownership is clear.
- Use `std::unique_ptr<T>` for single ownership.
- Use `std::shared_ptr<T>` only when ownership is genuinely shared.
- Prefer constructors and destructors over manual `init` and `destroy` calls.
- Prefer RAII wrappers for files, sockets, locks, memory buffers, and handles.

## Error Handling

- Decide whether the C++ layer should preserve C return codes or convert errors.
- If using exceptions, do not let them cross a C ABI boundary.
- If preserving return codes, consider `enum class`, `std::error_code`, `std::optional`, or result structs.
- Make cleanup exception-safe with RAII before introducing exceptions.
- Replace `goto cleanup` patterns with destructors only after confirming cleanup order.

## Data Structures

- Plain C structs can often remain plain structs in C++.
- Add constructors only when they clarify valid initialization.
- Avoid converting every struct into a class automatically.
- Replace manual dynamic arrays with `std::vector` when resizing or ownership is involved.
- Replace manual string buffers with `std::string` when the code owns text.
- Use `std::span` or pointer plus size for non-owning views, depending on the C++ standard available.

## Macros

- Replace constants with `constexpr` where possible.
- Replace small function-like macros with `inline` functions or templates.
- Keep macros for conditional compilation and platform-specific attributes.
- Watch for macros that conflict with C++ standard library names.

## Casting and Aliasing

- Replace C-style casts with the narrowest C++ cast:
  - `static_cast` for ordinary checked conversions.
  - `const_cast` only to change constness.
  - `reinterpret_cast` only for low-level representation work.
  - `dynamic_cast` only for polymorphic class hierarchies.
- Review strict aliasing and object lifetime assumptions.
- Be especially careful with code that writes bytes into raw storage and then treats the storage as an object.

## Concurrency

- C code may use platform threads, C11 threads, or custom synchronization.
- C++ code may use `std::thread`, `std::jthread`, `std::mutex`, atomics, and condition variables.
- Do not mix synchronization styles casually around the same shared state.
- Verify atomic type compatibility and memory order semantics.

## Testing Priorities

- Build the code as C first, then as C++ after each meaningful change.
- Enable warnings for both C and C++ modes.
- Add tests around allocation, cleanup, error paths, and boundary inputs.
- Use sanitizers when available: address, undefined behavior, thread.
- Add ABI tests if C callers will link against a C++ implementation.

## Common C to C++ Porting Bugs

- Missing `extern "C"` causes link errors.
- `malloc` result assigned without a cast fails in C++.
- C++ keyword used as a variable or field name.
- C flexible array member or variable length array rejected.
- C designated initializer rejected by older C++ standard.
- `sizeof('a')` changes.
- Function declaration `int f();` means something different.
- Error cleanup code becomes unsafe after exceptions are introduced.
- Memory allocated by one family is released by another.
- A raw pointer was treated as owning in one place and non-owning in another.
