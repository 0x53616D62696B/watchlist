# C++ Data Types and Containers

This guide compares common C++ data types and standard containers, with emphasis on practical selection: when to use `vector`, `list`, maps, sets, strings, pointers, and value types.

## Quick Choice Table

| Need | Prefer | Why |
| --- | --- | --- |
| Resizable array | `std::vector<T>` | Fast iteration, compact memory, random access |
| Fixed-size array known at compile time | `std::array<T, N>` | No heap allocation, size is part of the type |
| Non-owning view of contiguous values | `std::span<T>` | Pass array/vector data without copying |
| Text that owns its memory | `std::string` | Safer and easier than raw character buffers |
| Non-owning text view | `std::string_view` | Cheap parameter type for read-only text |
| Fast push/pop at both ends | `std::deque<T>` | Stable growth at front and back |
| Frequent insertion/removal in middle when you already have an iterator | `std::list<T>` | Node links make insertion/removal cheap |
| Key-value lookup by key | `std::unordered_map<K, V>` | Usually fastest average lookup |
| Ordered key-value lookup | `std::map<K, V>` | Keeps keys sorted |
| Unique values with fast lookup | `std::unordered_set<T>` | Hash-based membership check |
| Unique values in sorted order | `std::set<T>` | Ordered membership check |
| Queue behavior | `std::queue<T>` | First in, first out adapter |
| Stack behavior | `std::stack<T>` | Last in, first out adapter |
| Priority ordering | `std::priority_queue<T>` | Efficient access to largest/smallest item |
| Optional value | `std::optional<T>` | Represents "maybe present" without sentinel values |
| One of several possible types | `std::variant<A, B>` | Type-safe tagged union |
| Shared read-only constant | `constexpr` / `const` | Communicates immutability |
| Exclusive heap ownership | `std::unique_ptr<T>` | Single owner, automatic cleanup |
| Shared heap ownership | `std::shared_ptr<T>` | Multiple owners, reference-counted cleanup |

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

## `std::vector`

`std::vector<T>` is the default sequence container for most C++ code.

Strengths:

- Fast iteration because elements are contiguous in memory.
- Random access with `items[i]` in constant time.
- Works well with CPU caches.
- Simple ownership model.
- Compatible with APIs expecting contiguous data through `data()` and `size()`.

Costs:

- Inserting or removing from the middle shifts later elements.
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

## `std::array`

`std::array<T, N>` is a fixed-size array wrapper.

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
| Size | fixed at compile time | dynamic | dynamic | dynamic |
| Memory layout | contiguous | contiguous | segmented blocks | separate linked nodes |
| Random access | fast | fast | fast | not supported |
| Insert/remove at end | not resizable | fast amortized | fast | fast |
| Insert/remove at front | not resizable | slow because elements shift | fast | fast if position is known |
| Insert/remove in middle | not resizable | slow because later elements shift | slow | fast if iterator is already known |
| Iteration speed | excellent | usually excellent | good | usually poor |
| Cache locality | excellent | good | good, but less than vector | poor |
| Memory overhead | none beyond elements | low, may reserve extra capacity | moderate | high, each node stores links |
| Iterator/reference stability | stable for object lifetime | growth and erase can invalidate | usually more stable than vector, but rules are operation-specific | strong for existing elements |
| Main weakness | cannot resize | front/middle edits shift elements | not one contiguous buffer | slow traversal and no indexing |
| Best use | small fixed-size collections | default dynamic sequence | queues, buffers, push/pop at both ends | stable iterators and frequent splicing |

Rule of thumb:

- Use `std::array` when the size is known and never changes.
- Use `std::vector` for most runtime-sized lists.
- Use `std::deque` when both front and back operations matter.
- Use `std::list` only when stable iterators or node splicing are the real reason.

Default to `std::vector`. Choose `std::list` only when node-based behavior is part of the requirement.

## Maps

Use maps when each value is associated with a key.

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

| Adapter | Behavior | Common Operations |
| --- | --- | --- |
| `std::stack<T>` | last in, first out | `push`, `pop`, `top` |
| `std::queue<T>` | first in, first out | `push`, `pop`, `front`, `back` |
| `std::priority_queue<T>` | highest priority first | `push`, `pop`, `top` |

Use adapters when the restricted interface communicates intent clearly. Use the underlying containers directly when you need iteration or more control.

## Strings and Text Views

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

```cpp
#include <string>
#include <variant>

using Id = std::variant<int, std::string>;
```

Use it when a value can be one of several known alternatives. Prefer it over `void*`, manual type tags, or unsafe unions for ordinary application code.

## `std::span`

`std::span<T>` is a non-owning view over contiguous values.

```cpp
#include <span>
#include <vector>

void process(std::span<const int> values);

std::vector<int> numbers = {1, 2, 3};
process(numbers);
```

Use it for function parameters that accept arrays, vectors, or other contiguous buffers without taking ownership.

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
