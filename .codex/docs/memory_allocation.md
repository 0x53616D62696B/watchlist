# Memory Allocation in C++

This note explains where C++ objects live, what RAII means, and how arena and pool allocators are used.

## Storage Kinds

| Kind | Example | Lifetime | Common Use |
| --- | --- | --- | --- |
| Automatic storage | `int x = 42;` | scope exit | small local values |
| Dynamic storage | `new int{42}` | until deleted | objects that outlive a scope |
| Static storage | `static int x;` | whole program | globals, singletons, shared tables |
| Thread storage | `thread_local int x;` | current thread | per-thread state |

Automatic storage is usually implemented with the stack. Dynamic storage is usually called the heap.

```cpp
void example() {
    int stack_value = 42;
    auto heap_value = std::make_unique<int>(42);
}
```

`stack_value` is cleaned up when the function returns. `heap_value` owns dynamic memory and releases it automatically when the `std::unique_ptr` is destroyed.

## Volatile and Direct Memory Access

`volatile` does not allocate memory and it does not control ownership.

It tells the compiler that every read and write must really happen, because the value may change outside normal C++ code.

This is used mostly in embedded programming for direct access to memory-mapped hardware registers:

```cpp
#include <cstdint>

constexpr std::uintptr_t status_register_address = 0x40000000;

volatile std::uint32_t* status_register =
    reinterpret_cast<volatile std::uint32_t*>(status_register_address);

std::uint32_t status = *status_register;
```

Here, the program is not allocating memory at `0x40000000`. It is treating that existing hardware address as a register.

Without `volatile`, the compiler may cache, remove, merge, or reorder accesses because it cannot see that hardware may change the value.

Typical uses:

- memory-mapped hardware registers
- embedded device control/status registers
- values changed by interrupt handlers
- special low-level memory where every load or store matters

Do not use `volatile` for normal thread synchronization. For communication between C++ threads, use `std::atomic`, mutexes, condition variables, or other synchronization tools.

## RAII

RAII means Resource Acquisition Is Initialization.

The idea: acquire a resource in a constructor, release it in a destructor, and let object lifetime control cleanup.

```cpp
#include <cstdio>
#include <memory>

struct FileCloser {
    void operator()(std::FILE* file) const {
        if (file) {
            std::fclose(file);
        }
    }
};

using FilePtr = std::unique_ptr<std::FILE, FileCloser>;

FilePtr open_file(const char* path) {
    return FilePtr(std::fopen(path, "r"));
}
```

RAII is used for memory, files, sockets, mutex locks, database handles, graphics handles, and almost any resource that must be released.

Prefer:

- `std::vector` instead of manual dynamic arrays.
- `std::string` instead of manually owned character buffers.
- `std::unique_ptr` for single ownership.
- `std::shared_ptr` only when ownership is truly shared.
- stack objects when scope-based lifetime is enough.

Avoid:

```cpp
int* value = new int{42};
delete value;
```

Prefer:

```cpp
auto value = std::make_unique<int>(42);
```

## Arena Allocator

An arena allocator owns a large block of memory and gives out smaller allocations from that block.

Often, individual objects are not freed one by one. Instead, the whole arena is reset or destroyed at once.

This is useful when many objects have the same lifetime:

- parsing a file
- building a temporary graph
- per-frame game allocations
- request-local server data
- scratch memory for algorithms

Simple bump arena:

```cpp
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <vector>

class Arena {
public:
    explicit Arena(std::size_t size)
        : storage_(size) {}

    void* allocate(std::size_t size, std::size_t alignment) {
        void* current = storage_.data() + offset_;
        std::size_t remaining = storage_.size() - offset_;

        void* aligned = std::align(alignment, size, current, remaining);
        if (!aligned) {
            throw std::bad_alloc();
        }

        offset_ = storage_.size() - remaining + size;
        return aligned;
    }

    void reset() {
        offset_ = 0;
    }

private:
    std::vector<std::byte> storage_;
    std::size_t offset_ = 0;
};
```

Constructing an object inside the arena:

```cpp
struct Node {
    int value;
};

Arena arena(1024);

void* memory = arena.allocate(sizeof(Node), alignof(Node));
Node* node = new (memory) Node{42};
```

Important: placement `new` constructs the object, but it does not make the arena call destructors automatically.

If the type has a meaningful destructor, call it before resetting or destroying the arena:

```cpp
node->~Node();
arena.reset();
```

For trivially destructible types like simple integers or plain structs, this is often less important. For `std::string`, `std::vector`, file handles, locks, and other owning types, destruction matters.

## Pool Allocator

A pool allocator is optimized for many allocations of the same size.

It keeps a list of fixed-size blocks. Allocation pops one block from the free list. Deallocation pushes it back.

This is useful for:

- linked list nodes
- tree nodes
- particles
- messages
- fixed-size jobs/tasks
- objects allocated and freed frequently

Simple pool for one type:

```cpp
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <vector>

template <typename T>
class ObjectPool {
public:
    explicit ObjectPool(std::size_t capacity)
        : storage_(capacity) {
        for (auto& slot : storage_) {
            slot.next = free_;
            free_ = &slot;
        }
    }

    template <typename... Args>
    T* create(Args&&... args) {
        if (!free_) {
            throw std::bad_alloc();
        }

        Slot* slot = free_;
        free_ = free_->next;
        return new (&slot->object) T(std::forward<Args>(args)...);
    }

    void destroy(T* object) {
        if (!object) {
            return;
        }

        object->~T();
        Slot* slot = reinterpret_cast<Slot*>(object);
        slot->next = free_;
        free_ = slot;
    }

private:
    union Slot {
        Slot* next;
        alignas(T) std::byte object[sizeof(T)];
    };

    std::vector<Slot> storage_;
    Slot* free_ = nullptr;
};
```

Usage:

```cpp
struct Particle {
    float x;
    float y;

    Particle(float x_value, float y_value)
        : x(x_value), y(y_value) {}
};

ObjectPool<Particle> pool(1000);

Particle* p = pool.create(1.0f, 2.0f);
pool.destroy(p);
```

## Arena vs Pool

| Allocator | Best When | Freeing Model |
| --- | --- | --- |
| Arena | many objects die together | reset all at once |
| Pool | many same-size objects churn | free individual objects |

Use an arena when lifetime is simple and bulk cleanup is acceptable.

Use a pool when objects are created and destroyed individually but have the same type or size.

## Practical Rules

- Prefer normal RAII containers first: `std::vector`, `std::string`, `std::unique_ptr`.
- Use arenas when allocation speed and shared lifetime matter.
- Use pools when same-size objects are allocated and freed repeatedly.
- Do not mix allocation families: `new` pairs with `delete`, `malloc` pairs with `free`.
- Placement `new` needs manual destructor calls unless another owner handles destruction.
- Custom allocators are powerful, but they make lifetime bugs easier, so keep their scope small.
