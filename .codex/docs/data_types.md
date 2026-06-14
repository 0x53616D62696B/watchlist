# C++ Data Types and Containers

This guide compares common C++ data types and standard containers, with emphasis on practical selection: when to use `vector`, `list`, maps, sets, strings, pointers, and value types.

## Quick Choice Table

| Need | Prefer | Syntax | Why |
| --- | --- | --- | --- |
| Resizable array | `std::vector<T>` | `std::vector<int> v = {1, 2, 3};` | Fast iteration, compact memory, random access |
| Fixed-size array known at compile time | `std::array<T, N>` | `std::array<int, 3> a = {1, 2, 3};` | No heap allocation, size is part of the type |
| Non-owning view of contiguous values | `std::span<T>` | `std::span<const int> values;` | Pass array/vector data without copying |
| Text that owns its memory | `std::string` | `std::string name = "Ada";` | Safer and easier than raw character buffers |
| Non-owning text view | `std::string_view` | `std::string_view name = "Ada";` | Cheap parameter type for read-only text |
| Fast push/pop at both ends | `std::deque<T>` | `std::deque<int> d;` | Stable growth at front and back |
| Frequent insertion/removal in middle when you already have an iterator | `std::list<T>` | `std::list<int> l = {1, 2, 3};` | Node links make insertion/removal cheap |
| Key-value lookup by key | `std::unordered_map<K, V>` | `std::unordered_map<std::string, int> m;` | Usually fastest average lookup |
| Ordered key-value lookup | `std::map<K, V>` | `std::map<std::string, int> m;` | Keeps keys sorted |
| Unique values with fast lookup | `std::unordered_set<T>` | `std::unordered_set<int> s;` | Hash-based membership check |
| Unique values in sorted order | `std::set<T>` | `std::set<int> s;` | Ordered membership check |
| Queue behavior | `std::queue<T>` | `std::queue<int> q;` | First in, first out adapter |
| Stack behavior | `std::stack<T>` | `std::stack<int> s;` | Last in, first out adapter |
| Priority ordering | `std::priority_queue<T>` | `std::priority_queue<int> q;` | Efficient access to largest/smallest item |
| Optional value | `std::optional<T>` | `std::optional<int> value;` | Represents "maybe present" without sentinel values |
| One of several possible types | `std::variant<A, B>` | `std::variant<int, std::string> id;` | Type-safe tagged union |
| Shared read-only constant | `constexpr` / `const` | `constexpr int max = 10;` | Communicates immutability |
| Exclusive heap ownership | `std::unique_ptr<T>` | `std::unique_ptr<int> p;` | Single owner, automatic cleanup |
| Shared heap ownership | `std::shared_ptr<T>` | `std::shared_ptr<int> p;` | Multiple owners, reference-counted cleanup |

## Stack and Heap Placement

Placement depends on where the object is declared. A local variable usually lives on the stack; an object created with `new`, `std::make_unique`, or `std::make_shared` lives on the heap. Some C++ objects also manage separate heap memory internally.

| Type | Object Placement | Internal Data Placement |
| --- | --- | --- |
| fundamental values like `int`, `bool`, `double`, `char` | wherever declared | stored directly in the object |
| `enum class` | wherever declared | stored directly in the object |
| `std::array<T, N>` | wherever declared | elements are inside the array object |
| `std::vector<T>` | wherever declared | elements are usually heap-allocated |
| `std::deque<T>` | wherever declared | elements are stored in heap-allocated blocks |
| `std::list<T>` | wherever declared | each node is usually heap-allocated |
| `std::forward_list<T>` | wherever declared | each node is usually heap-allocated |
| `std::unordered_map<K, V>` | wherever declared | buckets and nodes are usually heap-allocated |
| `std::map<K, V>` | wherever declared | tree nodes are usually heap-allocated |
| `std::unordered_set<T>` | wherever declared | buckets and nodes are usually heap-allocated |
| `std::set<T>` | wherever declared | tree nodes are usually heap-allocated |
| `std::queue<T>` / `std::stack<T>` | wherever declared | depends on the underlying container |
| `std::priority_queue<T>` | wherever declared | usually uses a `std::vector`, so elements are usually heap-allocated |
| `std::string` | wherever declared | small text may be inside the object; larger text is usually heap-allocated |
| `std::string_view` | wherever declared | no owned data; points at someone else's text |
| `std::span<T>` | wherever declared | no owned data; points at someone else's contiguous elements |
| `std::optional<T>` | wherever declared | contained value is stored inside the optional object when present |
| `std::variant<A, B>` | wherever declared | active value is stored inside the variant object |
| `std::unique_ptr<T>` | wherever declared | pointed-to object is usually on the heap |
| `std::shared_ptr<T>` | wherever declared | pointed-to object and control block are usually on the heap |
| raw pointer `T*` | pointer variable lives wherever declared | pointed-to object can be stack, heap, static, or null |
| reference `T&` | alias to an existing object | does not own or place data |

## Fundamental Types

| Type | Typical Use | Notes |
| --- | --- | --- |
| `bool` | true/false state | Do not use integer flags when a boolean is enough |
| `char` | byte-sized character data | Signedness can be implementation-defined |
| `int` | general integer arithmetic | Size is at least 16 bits, commonly 32 bits |
| `std::size_t` | sizes and indexes | Unsigned; be careful when subtracting |
| `float` | smaller floating-point values | Lower precision than `double` |
| `double` | default floating-point choice | Usually the practical default for real numbers |
| `void` | no value or generic pointer base | `void*` loses type information |
| enum class | named finite choices | Prefer over plain `enum` for type safety |

Prefer fixed-width integer types from `<cstdint>` when exact size matters:

```cpp
#include <cstdint>

std::int32_t id = 42;
std::uint64_t mask = 0xff00;
```

Use plain `int` when the exact binary size does not matter. Use fixed-width types for file formats, network protocols, serialization, bit manipulation, and ABI-sensitive code.

Memory placement:

- A local fundamental value is usually stored directly on the stack.
- A fundamental value inside a heap object is stored as part of that heap object.
- A dynamically allocated fundamental value, such as `auto p = std::make_unique<int>(42);`, stores the `int` on the heap.

## `std::vector`

`std::vector<T>` is the default sequence container for most C++ code.

Memory placement:

```cpp
std::vector<int> v = {1, 2, 3};
```

Usually:

- the small vector control object is on the stack
- the actual elements are in dynamically allocated memory, usually heap

If you allocate the vector itself dynamically:

```cpp
auto v = std::make_unique<std::vector<int>>();
```

then both are heap-related:

- the vector object is on the heap
- its elements are also in heap allocation managed by the vector

Strengths:

- Fast iteration because elements are contiguous in memory.
- Random access with `items[i]` in constant time.
- Works well with CPU caches.
- Simple ownership model.
- Compatible with APIs expecting contiguous data through `data()` and `size()`.

Costs:

- Inserting or removing from the middle [shifts later elements](#shifting).
- Growing past capacity may reallocate and move all elements.
- References, pointers, and iterators can be invalidated by growth or erase.

Use it when:

- You need a dynamic list of values.
- You mostly append, iterate, sort, or index by position.
- You do not have a specific reason to use another container.

```cpp
#include <vector>

std::vector<int> numbers;
numbers.push_back(10);
numbers.push_back(20);

for (int value : numbers) {
    // use value
}
```

### Shifting

A std::vector must keep its elements packed together in one contiguous block:
`[A][B][C][D][E]`
If you remove C, vector shifts everything after it one position left:
Before:
`[A][B][C][D][E]`

Remove C:
`[A][B][D][E]`
Internally, D moves into C’s old position, and E moves into D’s old position. The vector’s size() becomes smaller.
There may still be unused capacity at the end, but not a hole in the middle:
`[A][B][D][E][unused capacity]`
For insertion, it does the opposite

## `std::array`

`std::array<T, N>` is a fixed-size array wrapper.

Memory placement:

```cpp
std::array<int, 3> values = {1, 2, 3};
```

Usually:

- the array object is on the stack when declared as a local variable
- the elements live inside the array object itself
- there is no separate heap allocation for the elements

If the array is part of a heap object or allocated dynamically, its elements are there with it:

```cpp
auto values = std::make_unique<std::array<int, 3>>();
```

Strengths:

- No dynamic allocation.
- Knows its own size.
- Works with range-based loops and standard algorithms.
- Safer and more expressive than raw arrays.

Costs:

- Size cannot change at runtime.
- Large arrays stored as local variables can use significant stack space.

```cpp
#include <array>

std::array<int, 3> values = {1, 2, 3};
```

Use `std::array` when the size is a true compile-time property. Use `std::vector` when the size depends on runtime input.

## `std::deque`

`std::deque<T>` is a double-ended queue.

Memory placement:

- the deque object lives wherever it is declared
- its elements are stored in dynamically allocated blocks, usually on the heap
- unlike `std::vector`, those elements are not stored in one single contiguous allocation

Strengths:

- Fast push and pop at the front and back.
- Random access is available.
- Growth usually does not move all elements.

Costs:

- Not stored as one contiguous block.
- Iteration is usually less cache-friendly than `std::vector`.

Use it when:

- You need efficient `push_front` and `pop_front`.
- You are implementing work queues, sliding windows, or buffers that grow at both ends.

```cpp
#include <deque>

std::deque<int> queue;
queue.push_back(1);
queue.push_front(0);
```

## `std::list`

`std::list<T>` is a doubly linked list.

Memory placement:

- the list object lives wherever it is declared
- each element is stored in a separate node, usually allocated on the heap
- each node also stores links to neighboring nodes, which adds memory overhead

Strengths:

- Fast insertion and removal when you already have an iterator to the position.
- Existing elements are not moved during insertion or removal.
- Iterators to other elements usually remain valid.

Costs:

- No random access with `items[i]`.
- Each element allocates a separate node.
- Poor cache locality compared with `std::vector`.
- Usually slower in practice unless its specific strengths matter.

Use it when:

- You frequently splice, insert, or remove elements from the middle.
- You already have iterators to the affected elements.
- Stable iterators are important.

Avoid it when:

- You only need a general-purpose list.
- You mainly iterate, append, sort, or access by index.

```cpp
#include <list>

std::list<int> values = {1, 3};
auto it = values.begin();
++it;
values.insert(it, 2);
```

## `std::forward_list`

`std::forward_list<T>` is a singly linked list.

Memory placement:

- the forward list object lives wherever it is declared
- each element is stored in a separate node, usually allocated on the heap
- each node stores only a next link, so it has less overhead than `std::list`

Strengths:

- Lower per-node overhead than `std::list`.
- Fast insertion/removal after a known position.

Costs:

- Forward iteration only.
- No `size()` in older standards.
- Less convenient than `std::list`.

Use it only when you specifically need a minimal singly linked list. Most code should prefer `std::vector`, `std::deque`, or `std::list`.

## Vector vs Array vs Deque vs List

| Property | `std::array<T, N>` | `std::vector<T>` | `std::deque<T>` | `std::list<T>` |
| --- | --- | --- | --- | --- |
| **Size** | fixed at compile time | dynamic | dynamic | dynamic |
| **Memory layout** | contiguous | contiguous | segmented blocks | separate linked nodes |
| **Random access** | fast | fast | fast | not supported |
| **Insert/remove at end** | not resizable | fast amortized | fast | fast |
| **Insert/remove at front** | not resizable | slow because elements shift | fast | fast if position is known |
| **Insert/remove in middle** | not resizable | slow because later elements shift | slow | fast if iterator is already known |
| **Iteration speed** | excellent | usually excellent | good | usually poor |
| **Cache locality** | excellent | good | good, but less than vector | poor |
| **Memory overhead** | none beyond elements | low, may reserve extra capacity | moderate | high, each node stores links |
| **Iterator/reference stability** | stable for object lifetime | growth and erase can invalidate | usually more stable than vector, but rules are operation-specific | strong for existing elements |
| **Main weakness** | cannot resize | front/middle edits shift elements | not one contiguous buffer | slow traversal and no indexing |
| **Best use** | small fixed-size collections | default dynamic sequence | queues, buffers, push/pop at both ends | stable iterators and frequent splicing |
| **Closest Python equivalent** | `tuple` or fixed-length `list` by convention | `list` | `collections.deque` | no common built-in equivalent |

Legend:

- **Fast amortized**: usually fast on average across many operations, but one operation can occasionally be slower. For `std::vector::push_back`, this can happen when the vector grows and moves existing elements.
- **Cache locality**: how easily the CPU can read nearby data efficiently. How close elements are in memory.

  Modern CPUs do not usually fetch only one variable from RAM. They fetch a small block of nearby memory into fast CPU cache. So if your data is stored next to each other, the CPU can process it very quickly.

Rule of thumb:

- Use `std::array` when the size is known and never changes.
- Use `std::vector` for most runtime-sized lists.
- Use `std::deque` when both front and back operations matter.
- Use `std::list` only when stable iterators or node splicing are the real reason.

Default to `std::vector`. Choose `std::list` only when node-based behavior is part of the requirement.

## Maps

Use maps when each value is associated with a key.

Memory placement:

- the map object lives wherever it is declared
- `std::unordered_map` usually stores buckets and key-value nodes on the heap
- `std::map` usually stores each key-value pair in a heap-allocated tree node

| Container | Ordering | Lookup | Best For |
| --- | --- | --- | --- |
| `std::unordered_map<K, V>` | no sorted order | average constant time | fast lookup by key |
| `std::map<K, V>` | sorted by key | logarithmic time | ordered traversal and range queries |

```cpp
#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> counts;
counts["apple"] += 1;
```

Prefer `std::unordered_map` when ordering does not matter. Prefer `std::map` when sorted keys or ordered range operations matter.

Important notes:

- `operator[]` inserts a default value when the key does not exist.
- Use `find`, `contains`, or `at` when accidental insertion would be a bug.
- Custom key types need hashing for `unordered_map` or comparison for `map`.

## Sets

Use sets when you need unique values without separate mapped data.

Memory placement:

- the set object lives wherever it is declared
- `std::unordered_set` usually stores buckets and value nodes on the heap
- `std::set` usually stores each value in a heap-allocated tree node

| Container | Ordering | Lookup | Best For |
| --- | --- | --- | --- |
| `std::unordered_set<T>` | no sorted order | average constant time | fast membership checks |
| `std::set<T>` | sorted by value | logarithmic time | ordered unique values |

```cpp
#include <string>
#include <unordered_set>

std::unordered_set<std::string> seen;

if (!seen.contains("alpha")) {
    seen.insert("alpha");
}
```

## Container Adapters

Container adapters expose restricted interfaces for common data structures.

Memory placement:

- the adapter object lives wherever it is declared
- stored elements live wherever the underlying container stores them
- `std::stack` and `std::queue` commonly use `std::deque` by default
- `std::priority_queue` commonly uses `std::vector` by default

| Adapter | Behavior | Common Operations |
| --- | --- | --- |
| `std::stack<T>` | last in, first out | `push`, `pop`, `top` |
| `std::queue<T>` | first in, first out | `push`, `pop`, `front`, `back` |
| `std::priority_queue<T>` | highest priority first | `push`, `pop`, `top` |

Use adapters when the restricted interface communicates intent clearly. Use the underlying containers directly when you need iteration or more control.

## Strings and Text Views

Memory placement:

- `std::string` object lives wherever it is declared
- small strings may be stored inside the string object itself
- larger strings are usually stored in heap memory managed by the string
- `std::string_view` does not own text; it only stores a pointer and length to someone else's characters
- `const char*` points to existing characters, such as a string literal or C buffer
- `char[]` stores characters directly inside the array object

| Type | Owns Memory | Nullable | Best For |
| --- | --- | --- | --- |
| `std::string` | yes | no | stored and mutable text |
| `std::string_view` | no | no | read-only function parameters |
| `const char*` | no | yes | C APIs and string literals |
| `char[]` | yes, fixed buffer | no | low-level mutable buffers |

```cpp
#include <string>
#include <string_view>

void print_name(std::string_view name);

std::string owned = "Ada";
print_name(owned);
print_name("Grace");
```

The important danger: `std::string_view` does not own the text. The original characters must outlive the view.

## Pointers and Ownership

Memory placement:

- a raw pointer variable lives wherever it is declared, but the object it points to can be on the stack, heap, static storage, or null
- a reference is only an alias to an existing object; it does not allocate or own memory
- `std::unique_ptr<T>` object lives wherever it is declared, while the object it owns is usually on the heap
- `std::shared_ptr<T>` object lives wherever it is declared, while the shared object and reference-count control block are usually on the heap

| Type | Meaning |
| --- | --- |
| `T*` | raw pointer, usually non-owning or C-compatible |
| `T&` | required non-null reference |
| `std::unique_ptr<T>` | exclusive ownership |
| `std::shared_ptr<T>` | shared ownership |
| `std::weak_ptr<T>` | non-owning reference to shared ownership |

Prefer values and containers first. Use smart pointers when dynamic lifetime is required. Use raw pointers for non-owning observation, optional parameters, and C APIs.

```cpp
#include <memory>

auto item = std::make_unique<int>(42);
```

## `std::optional`

`std::optional<T>` represents a value that may or may not exist.

Memory placement:

- the optional object lives wherever it is declared
- when a value is present, that value is stored inside the optional object
- `std::optional<T>` does not normally allocate heap memory by itself
- if `T` manages heap memory, then that internal behavior still applies

```cpp
#include <optional>

std::optional<int> find_index();

if (auto index = find_index()) {
    // use *index
}
```

Use it instead of magic values like `-1`, empty strings, or null pointers when absence is a valid result.

## `std::variant`

`std::variant<A, B, C>` stores exactly one value from a fixed set of types.

Memory placement:

- the variant object lives wherever it is declared
- the active value is stored inside the variant object
- `std::variant` does not normally allocate heap memory by itself
- if the active type manages heap memory, such as `std::string`, then that type may allocate internally

```cpp
#include <string>
#include <variant>

using Id = std::variant<int, std::string>;
```

Use it when a value can be one of several known alternatives. Prefer it over `void*`, manual type tags, or unsafe unions for ordinary application code.

## `std::span`

`std::span<T>` is a non-owning view over contiguous values.

Memory placement:

- the span object lives wherever it is declared
- it does not own or allocate the elements
- it stores only a pointer and size pointing to someone else's contiguous memory

```cpp
#include <span>
#include <vector>

void process(std::span<const int> values);

std::vector<int> numbers = {1, 2, 3};
process(numbers);
```

Use it for function parameters that accept arrays, vectors, or other contiguous buffers without taking ownership.

## Python Similarities

These are similarities, not exact replacements. Python types are dynamically typed and usually store references to objects. C++ containers have explicit element types, ownership behavior, memory layout, and iterator invalidation rules.

| C++ Type | C++ Syntax | Closest Python Idea | Important Difference |
| --- | --- | --- | --- |
| `bool` | `bool ok = true;` | `bool` | Very similar for true/false logic |
| `char` | `char c = 'A';` | one-character `str` or small `bytes` value | C++ `char` is a small numeric character/byte type |
| `int` | `int count = 42;` | `int` | Python `int` grows automatically; C++ integer size is fixed |
| `std::size_t` | `std::size_t i = 0;` | non-negative `int` | C++ type is unsigned and used for sizes/indexes |
| `float` | `float x = 1.5f;` | `float` | Python `float` is closer to C++ `double` |
| `double` | `double x = 1.5;` | `float` | Python's normal floating-point type is double precision |
| `void` | `void log();` | `None` for no return value | C++ `void` is a type meaning "no value" |
| `enum class` | `enum class Color { Red, Blue };` | `enum.Enum` | C++ enum classes are compile-time typed values |
| `std::array<T, N>` | `std::array<int, 3> a = {1, 2, 3};` | `tuple` or fixed-length `list` by convention | C++ size is fixed by the type |
| `std::vector<T>` | `std::vector<int> v = {1, 2, 3};` | `list` | C++ vector stores one element type contiguously |
| `std::deque<T>` | `std::deque<int> d;` | `collections.deque` | Both are good for front/back operations |
| `std::list<T>` | `std::list<int> l = {1, 2, 3};` | no common built-in equivalent | Python `list` is closer to `std::vector`, not linked list |
| `std::forward_list<T>` | `std::forward_list<int> l = {1, 2, 3};` | no common built-in equivalent | Singly linked lists are uncommon in normal Python code |
| `std::unordered_map<K, V>` | `std::unordered_map<std::string, int> m;` | `dict` | C++ key and value types are explicit |
| `std::map<K, V>` | `std::map<std::string, int> m;` | sorted dictionary-like mapping | Python `dict` preserves insertion order, not sorted key order |
| `std::unordered_set<T>` | `std::unordered_set<int> s;` | `set` | Both are hash-based unique collections |
| `std::set<T>` | `std::set<int> s;` | sorted set-like collection | Python built-in `set` is not sorted |
| `std::queue<T>` | `std::queue<int> q;` | `queue.Queue` or `collections.deque` | C++ queue is an adapter with a restricted interface |
| `std::stack<T>` | `std::stack<int> s;` | `list` used with `append`/`pop` | C++ stack intentionally hides iteration/indexing |
| `std::priority_queue<T>` | `std::priority_queue<int> q;` | `heapq` | Python `heapq` works on a list; C++ exposes a queue adapter |
| `std::string` | `std::string name = "Ada";` | `str` | Python `str` is immutable; C++ string is mutable |
| `std::string_view` | `std::string_view name = "Ada";` | slice/view concept | Python string slices create strings; C++ view borrows existing text |
| `std::span<T>` | `std::span<const int> values;` | view over a sequence | C++ span is non-owning and only works with contiguous data |
| `std::optional<T>` | `std::optional<int> value;` | value or `None` | C++ optional keeps the contained type explicit |
| `std::variant<A, B>` | `std::variant<int, std::string> id;` | union-like value or pattern matching | C++ variant restricts alternatives to listed types |
| `std::unique_ptr<T>` | `std::unique_ptr<int> p;` | no direct everyday equivalent | Python objects are reference-managed automatically |
| `std::shared_ptr<T>` | `std::shared_ptr<int> p;` | normal shared object references | C++ shared pointer makes shared ownership explicit |
| raw pointer `T*` | `int* p = nullptr;` | object reference or `None` | C++ pointers can be nullable and require lifetime discipline |
| reference `T&` | `int& ref = value;` | normal object reference | C++ references are non-null aliases with stricter rules |

## Common Rules of Thumb

- Start with `std::vector` for sequences.
- Use `std::array` for fixed-size collections.
- Use `std::unordered_map` or `std::unordered_set` for fast key/member lookup.
- Use ordered `std::map` or `std::set` only when sorted order matters.
- Use `std::deque` for efficient work at both ends.
- Use `std::list` only when iterator stability and middle insertion/removal are real requirements.
- Prefer `std::string` for owning text and `std::string_view` for read-only parameters.
- Prefer values, references, and smart pointers over raw owning pointers.
- Prefer `std::optional` over sentinel values.
- Prefer `enum class` over plain enums.

## Common Mistakes

- Choosing `std::list` because "linked lists are faster for insertion" without considering cache locality and iterator lookup cost.
- Using `std::map` when sorted order is not needed.
- Using `operator[]` on maps when missing keys should not create entries.
- Returning `std::string_view` to a temporary or local string.
- Storing pointers or references to `std::vector` elements across operations that may reallocate.
- Mixing signed `int` indexes with unsigned `std::size_t` sizes carelessly.
- Using raw `new` and `delete` instead of containers or smart pointers.
- Using inheritance or pointers when simple value types would be clearer.

## Best Mental Model

Use types to make ownership, lifetime, and lookup behavior obvious.

For sequences, ask: do I need contiguous memory, stable iterators, or efficient front/back operations?

For lookup, ask: do I need sorted order or just fast membership/key access?

For values, ask: does this object own data, borrow data, or maybe not exist?
