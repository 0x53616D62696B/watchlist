# Quick Reference

## Core Difference Table

| Topic | C | C++ |
| --- | --- | --- |
| Main style | procedural | multi-paradigm |
| Encapsulation | opaque structs and naming | classes, access control, namespaces |
| Generic code | macros, `void*`, code generation | templates, overloads, concepts in newer standards |
| Resource cleanup | explicit cleanup calls | RAII destructors plus explicit tools when needed |
| Dynamic memory | `malloc` / `free` | containers, smart pointers, `new` / `delete` |
| Error handling | return codes, `errno`, output params | return codes, exceptions, typed results |
| Function names | no overloads | overloads and name mangling |
| ABI boundary | simple and widely used | compiler and standard-library dependent |
| Strings | `char*`, arrays, length conventions | `std::string`, `std::string_view`, C strings |
| Arrays | raw arrays, pointer plus count | raw arrays, `std::array`, `std::vector`, `std::span` |
| Compile-time abstraction | preprocessor | `constexpr`, templates, type traits, preprocessor |

## C to C++ Checklist

- Does every shared header compile in both C and C++ modes?
- Do exported C symbols use `extern "C"` when included from C++?
- Are `malloc` / `free` pairs still paired correctly?
- Can ownership move to RAII, containers, or smart pointers?
- Are C99 features such as VLAs, flexible array members, compound literals, or broad designated initializers used?
- Are any identifiers C++ keywords?
- Are pointer conversions explicit and intentional?
- Are enum and integer conversions still valid?
- Does `int f();` mean what the code expects?
- Do errors still clean up correctly if exceptions are introduced?
- Are ABI-facing structs and functions still C-compatible?

## C++ to C Checklist

- Can the implementation remain C++ with a C wrapper instead?
- Which types own memory, handles, locks, or files?
- What replaces each constructor and destructor?
- What replaces exceptions?
- What replaces overloads?
- What replaces templates?
- What replaces classes and private invariants?
- What replaces `std::string`, `std::vector`, smart pointers, and other library types?
- Are pointer parameters nullable or required?
- Who allocates and who frees each returned object or buffer?
- Are all cleanup paths explicit and tested?
- Does the public header compile as C?

## Boundary Rules

- Do not let C++ exceptions cross a C ABI.
- Do not expose C++ standard library types in a C API.
- Do not expose C++ classes directly as a stable binary API unless all callers use the same compatible C++ toolchain.
- Do not mix `malloc` with `delete`.
- Do not mix `new` with `free`.
- Do not put C++-only declarations inside `extern "C"`.
- Prefer opaque handles at C boundaries.

## Practical Translation Patterns

| C++ Feature | C Pattern |
| --- | --- |
| class | `struct` plus prefixed functions |
| private data | opaque pointer |
| constructor | `init` or `create` |
| destructor | `destroy` or `close` |
| reference parameter | pointer parameter |
| overload | unique function names |
| exception | return code or result struct |
| template | macro, concrete type, or `void*` plus size |
| virtual function | function pointer table |
| lambda capture | callback plus `void* context` |
| namespace | symbol prefix |
| `std::vector` | pointer, count, capacity |
| `std::string` | pointer plus length or caller buffer |

## Small Examples

### C API Usable From C++

```c
#ifndef EXAMPLE_H
#define EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Example Example;

Example* example_create(void);
void example_destroy(Example* example);
int example_do_work(Example* example, int value);

#ifdef __cplusplus
}
#endif

#endif
```

### C++ RAII Wrapper Around a C Handle

```cpp
class ExampleHandle {
public:
    ExampleHandle() : handle_(example_create()) {}
    ~ExampleHandle() { example_destroy(handle_); }

    ExampleHandle(const ExampleHandle&) = delete;
    ExampleHandle& operator=(const ExampleHandle&) = delete;

    ExampleHandle(ExampleHandle&& other) noexcept
        : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    ExampleHandle& operator=(ExampleHandle&& other) noexcept {
        if (this != &other) {
            example_destroy(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    int do_work(int value) {
        return example_do_work(handle_, value);
    }

private:
    Example* handle_ = nullptr;
};
```

## Best Mental Model

When moving from C to C++, ask: which conventions can become types?

When moving from C++ to C, ask: which invisible guarantees must become explicit API rules?
