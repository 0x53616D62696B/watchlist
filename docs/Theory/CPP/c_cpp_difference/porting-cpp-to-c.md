# Porting C++ Code to C

Moving C++ code to C is usually a rewrite, not a mechanical translation. C does not have constructors, destructors, classes, templates, references, exceptions, overloads, namespaces, or the C++ standard library.

## First Decide the Target

Clarify the reason for moving to C:

- expose a stable C ABI
- support a C-only compiler or platform
- reduce runtime or toolchain dependencies
- interoperate with another language
- rewrite a C++ module into simpler procedural code

If the goal is interoperability, consider keeping the implementation in C++ and exposing a C wrapper API.

## Interface Design

C does not support overloads or namespaces. Use explicit names.

```c
watchlist_item_create(...)
watchlist_item_destroy(...)
watchlist_item_set_name(...)
watchlist_item_get_name(...)
```

For encapsulation, use opaque structs:

```c
typedef struct WatchlistItem WatchlistItem;

WatchlistItem* watchlist_item_create(void);
void watchlist_item_destroy(WatchlistItem* item);
```

The `.c` file defines the struct layout. Callers only see the handle.

## Classes and Objects

Translate classes into structs plus functions.

C++:

```cpp
class Buffer {
public:
    explicit Buffer(size_t size);
    ~Buffer();
    size_t size() const;
private:
    char* data_;
    size_t size_;
};
```

C:

```c
typedef struct Buffer {
    char* data;
    size_t size;
} Buffer;

int buffer_init(Buffer* buffer, size_t size);
void buffer_destroy(Buffer* buffer);
size_t buffer_size(const Buffer* buffer);
```

Choose one of these ownership styles:

- caller owns the struct storage and calls `init` / `destroy`
- library allocates the object and caller calls `create` / `destroy`

Document the choice clearly.

## Constructors and Destructors

C has no automatic constructors or destructors.

- Replace constructors with `init` or `create` functions.
- Replace destructors with `destroy`, `close`, or `free` functions.
- Make destroy functions tolerate partially initialized objects when possible.
- Define whether destroy functions accept `NULL`.
- Make cleanup order explicit.

RAII cleanup in C++ must become explicit cleanup in C.

## References

C has pointers, not references.

- Replace `T&` with `T*`.
- Replace `const T&` with `const T*` for non-owning access.
- Check nullability. C++ references cannot normally be null; C pointers can.
- Document whether each pointer may be `NULL`.

## Function Overloading

C has one global function namespace per linkage scope.

C++ overloads must become distinct names:

```cpp
draw(int x);
draw(const Shape& shape);
```

```c
draw_int(int x);
draw_shape(const Shape* shape);
```

## Templates

C has no templates.

Options:

- write a concrete implementation for each needed type
- use macros to generate type-specific code
- use `void*` plus element sizes and callbacks
- use code generation

Prefer concrete typed code when the set of types is small. Prefer callbacks and `void*` only when a generic C API is truly needed.

## Inheritance and Virtual Functions

C has no built-in inheritance or virtual dispatch.

Options:

- flatten the design into explicit functions
- use composition
- use tagged unions
- use function pointer tables

Manual vtable pattern:

```c
typedef struct ShapeVTable {
    double (*area)(const void* self);
    void (*destroy)(void* self);
} ShapeVTable;

typedef struct Shape {
    const ShapeVTable* vtable;
} Shape;
```

Use this only when runtime polymorphism is actually needed.

## Exceptions

C has no exceptions.

Replace exceptions with:

- return codes
- error enums
- output parameters
- result structs
- `errno` or thread-local error state, if appropriate

Avoid `setjmp` / `longjmp` unless the project already uses them and the constraints are well understood.

Important: C++ exceptions must not cross a C boundary. Catch them in the C++ wrapper and convert them to C errors.

## Standard Library Replacement

Map C++ library types intentionally.

| C++ | Common C Replacement |
| --- | --- |
| `std::string` | `char*` plus length, or caller-provided buffer |
| `std::string_view` | `const char*` plus length |
| `std::vector<T>` | pointer plus count and capacity |
| `std::array<T, N>` | fixed C array |
| `std::unique_ptr<T>` | explicit ownership convention |
| `std::shared_ptr<T>` | manual reference counting, if needed |
| `std::optional<T>` | status flag plus output value |
| `std::variant` | tagged union |
| `std::function` | function pointer plus context pointer |
| `std::map` | custom tree/hash table or sorted array |

## Lambdas and Callbacks

C has function pointers, but no capturing lambdas.

Use function pointer plus context:

```c
typedef void (*Callback)(void* context, int value);

void run_callback(Callback callback, void* context);
```

This replaces C++ captures with explicit user data.

## Namespaces

C has no namespaces.

- Use consistent prefixes.
- Keep public symbols short enough to read but specific enough to avoid collisions.
- Hide internal functions with `static`.
- Use visibility attributes or export maps when building shared libraries.

## Const Correctness

C has `const`, but without C++ member functions and overloads.

- Preserve `const` on pointer targets for read-only access.
- Review every API that returns internal memory.
- Document whether returned pointers remain valid after mutation or destruction.

## Memory Management

- Decide who allocates and who frees each object or buffer.
- Pair allocation and release APIs in the same module.
- Avoid requiring callers to use the wrong allocator family.
- If the library allocates memory, provide a matching library free function.
- Replace destructor-backed cleanup with explicit cleanup on every error path.

## Build and ABI

- Remove C++-only headers and library dependencies.
- Remove exceptions, RTTI, templates, classes, references, and overloads from C-facing headers.
- Ensure headers compile as C with the target C standard.
- Use `extern "C"` only in headers that may be included by C++ callers.
- Validate struct packing and ABI assumptions if binary compatibility matters.

## Testing Priorities

- Add tests for every init and cleanup path.
- Test allocation failures if the codebase supports fault injection.
- Test ownership transfer and double-destroy behavior.
- Test API behavior with `NULL` where allowed.
- Test error propagation because exception paths become manual branches.
- Run leak detection or sanitizers where available.

## Common C++ to C Porting Bugs

- Forgotten cleanup after replacing RAII.
- Lost error information after replacing exceptions.
- Pointer parameters accidentally accept `NULL` where references did not.
- Two C++ overloads collapse into one C symbol name.
- C++ object invariants are no longer enforced after moving to public structs.
- Template code becomes unsafe `void*` code without size and type checks.
- Capturing lambda loses its captured state when translated to a raw function pointer.
- C callers free memory with a different allocator than the library used.
