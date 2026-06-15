# Constructors and Initializers in C++ and Python

## Big Picture

Constructors and initializers prepare an object so it starts its lifetime in a valid state.

C++ and Python both support object-oriented programming, but they handle object creation differently:

- C++ constructs objects directly and cares deeply about memory layout, object lifetime, and initialization order.
- Python creates objects dynamically and then initializes their attributes through normal runtime code.

In C++, construction is tied to storage and lifetime.
In Python, construction is split between object allocation and object initialization.

## C++ Constructors

A C++ constructor is a special member function that runs when an object is created.
It has the same name as the class and no return type.

```cpp
class User {
public:
    User(std::string name, int age)
        : name_(std::move(name)), age_(age)
    {
    }

private:
    std::string name_;
    int age_;
};
```

When this object is created, C++ automatically calls the constructor:

```cpp
User user("Alice", 30);
```

The constructor's job is to establish the object's invariants.
An invariant is a condition that should always be true while the object is usable.

For example, a `Socket` object might guarantee that it either owns a valid socket handle or clearly represents an empty state.

## C++ Member Initializer List

C++ constructors often use a member initializer list.
This is the part after the colon:

```cpp
class Point {
public:
    Point(int x, int y)
        : x_(x), y_(y)
    {
    }

private:
    int x_;
    int y_;
};
```

The initializer list constructs members directly.
This is different from assigning values inside the constructor body.

```cpp
class Example {
public:
    Example(std::string value)
        : value_(std::move(value)) // direct initialization
    {
    }

private:
    std::string value_;
};
```

Assignment inside the constructor body happens after the member has already been initialized:

```cpp
class Example {
public:
    Example(std::string value)
    {
        value_ = std::move(value); // assignment after default construction
    }

private:
    std::string value_;
};
```

For simple types this distinction may not matter much, but for complex types it can affect performance and correctness.

Initializer lists are required for:

- `const` data members
- reference data members
- members without default constructors
- base class constructors

```cpp
class Config {
public:
    Config(std::string path)
        : path_(std::move(path))
    {
    }

private:
    const std::string path_;
};
```

The `const` member must be initialized before the constructor body starts.
It cannot be assigned later.

## C++ Initialization Order

C++ initializes members in the order they are declared in the class, not the order written in the initializer list.

```cpp
class Example {
public:
    Example()
        : second_(first_), first_(42)
    {
    }

private:
    int first_;
    int second_;
};
```

Even though `second_` appears first in the initializer list, `first_` is initialized first because it is declared first.

Best practice: write initializer lists in the same order as member declarations.

## Common C++ Constructor Types

### Default Constructor

A default constructor can be called without arguments.

```cpp
class Counter {
public:
    Counter() : value_(0) {}

private:
    int value_;
};
```

### Parameterized Constructor

A parameterized constructor accepts values from the caller.

```cpp
class Counter {
public:
    explicit Counter(int start) : value_(start) {}

private:
    int value_;
};
```

Use `explicit` for single-argument constructors unless implicit conversion is intentional.

### Copy Constructor

A copy constructor creates a new object from an existing object.

```cpp
class Counter {
public:
    Counter(const Counter& other) : value_(other.value_) {}

private:
    int value_;
};
```

### Move Constructor

A move constructor transfers resources from a temporary or expiring object.

```cpp
class Buffer {
public:
    Buffer(Buffer&& other) noexcept
        : data_(std::move(other.data_))
    {
    }

private:
    std::vector<int> data_;
};
```

Move constructors are important for efficient resource management.

## C++ RAII

C++ constructors are often paired with destructors through RAII.

RAII means Resource Acquisition Is Initialization.
The object acquires a resource in its constructor and releases it in its destructor.

```cpp
class File {
public:
    explicit File(const std::string& path)
        : handle_(std::fopen(path.c_str(), "r"))
    {
        if (!handle_) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~File()
    {
        std::fclose(handle_);
    }

private:
    std::FILE* handle_;
};
```

This makes resource lifetime deterministic.
When the object goes out of scope, cleanup happens automatically.

## Python Initializers

Python usually initializes objects with the `__init__` method.

```python
class User:
    def __init__(self, name: str, age: int) -> None:
        self.name = name
        self.age = age
```

Creating an object calls `__init__` after the object has already been allocated:

```python
user = User("Alice", 30)
```

Unlike C++, `__init__` is not the real constructor.
It is an initializer.

The first parameter, usually named `self`, refers to the instance being initialized.
Attributes are commonly created by assigning to `self`.

## Python `__new__` vs `__init__`

Python object creation has two main steps:

1. `__new__` creates and returns the object.
2. `__init__` initializes the object.

Most classes only need `__init__`.

```python
class User:
    def __init__(self, name: str) -> None:
        self.name = name
```

`__new__` is used for advanced cases, especially immutable types, singletons, or custom allocation behavior.

```python
class UppercaseString(str):
    def __new__(cls, value: str):
        return super().__new__(cls, value.upper())
```

Here, the value must be changed before the immutable `str` object is created.
That is why `__new__` is used instead of `__init__`.

## Python Attribute Initialization

Python attributes do not need to be declared before use.
They are usually added dynamically inside `__init__`.

```python
class Point:
    def __init__(self, x: int, y: int) -> None:
        self.x = x
        self.y = y
```

This flexibility is convenient, but it also means misspelled attributes can create bugs:

```python
class Point:
    def __init__(self, x: int, y: int) -> None:
        self.x = x
        self.y = y

    def move_right(self) -> None:
        self.xx = self.x + 1 # probably a bug
```

Type checkers, tests, dataclasses, and `__slots__` can help catch this kind of mistake.

## Python Dataclasses

Python dataclasses reduce boilerplate for classes that mostly store data.

```python
from dataclasses import dataclass


@dataclass
class User:
    name: str
    age: int
```

This automatically generates an `__init__` similar to:

```python
def __init__(self, name: str, age: int) -> None:
    self.name = name
    self.age = age
```

Dataclasses are useful when a class mainly represents structured data.
For more complex invariants, a hand-written `__init__` may still be clearer.

## Validation

Both C++ constructors and Python initializers can validate input.

C++ example:

```cpp
class Percentage {
public:
    explicit Percentage(int value)
        : value_(value)
    {
        if (value < 0 || value > 100) {
            throw std::out_of_range("percentage must be between 0 and 100");
        }
    }

private:
    int value_;
};
```

Python example:

```python
class Percentage:
    def __init__(self, value: int) -> None:
        if value < 0 or value > 100:
            raise ValueError("percentage must be between 0 and 100")
        self.value = value
```

One important difference is that C++ member initialization may already have happened before validation in the constructor body.
If validation must happen before constructing a member, use a helper function or factory.

## Side-by-Side Comparison

| Topic | C++ | Python |
| --- | --- | --- |
| Main mechanism | Constructor | `__init__` initializer |
| Actual allocation hook | Constructor is tied to object lifetime | `__new__` creates the object |
| Attribute/member declaration | Members are declared in the class | Attributes are usually assigned dynamically |
| Initialization order | Strict and declaration-based | Runtime assignment order inside `__init__` |
| Cleanup | Destructor and RAII | Garbage collection and context managers |
| Resource lifetime | Deterministic for stack objects | Usually non-deterministic unless using `with` |
| Common data-class tool | Structs/classes with constructors | `@dataclass` |
| Type enforcement | Compile-time type system | Runtime behavior, optional type checking |

## Resource Management Difference

C++ commonly uses constructors and destructors to manage resources:

```cpp
{
    std::lock_guard<std::mutex> lock(mutex);
    // mutex is locked here
}
// mutex is unlocked here
```

Python usually uses context managers for deterministic resource cleanup:

```python
with open("data.txt", "r") as file:
    data = file.read()
# file is closed here
```

Python has destructors through `__del__`, but they are not usually the right tool for important cleanup.
Prefer context managers with `with`.

## Common Mistakes

### C++ Mistakes

- Assigning members in the constructor body when they should be initialized in the initializer list.
- Writing initializer lists in a different order from member declarations.
- Forgetting `explicit` on constructors that should not allow implicit conversion.
- Doing complex work in constructors that makes errors hard to handle.
- Letting partially constructed objects leak resources.

### Python Mistakes

- Thinking `__init__` creates the object.
- Forgetting `self` when assigning attributes.
- Creating inconsistent attributes across different branches of `__init__`.
- Using mutable default arguments, such as `items=[]`.
- Relying on `__del__` for important cleanup.

Bad mutable default example:

```python
class Basket:
    def __init__(self, items=[]): # shared between calls
        self.items = items
```

Better:

```python
class Basket:
    def __init__(self, items: list[str] | None = None) -> None:
        self.items = [] if items is None else items
```

## Practical Summary

In C++, constructors are part of the language's object lifetime model.
They directly initialize members and base classes, and they are central to RAII.

In Python, `__init__` is normal runtime code that initializes an already-created object.
It is flexible and expressive, but it depends more on discipline, tests, and optional type checking.

Use C++ initializer lists when building C++ objects.
Use Python `__init__`, dataclasses, and context managers when building Python objects.

## Sources

- cppreference: [Constructors and member initializer lists](https://en.cppreference.com/w/cpp/language/constructor)
- cppreference: [Initialization](https://en.cppreference.com/w/cpp/language/initialization)
- cppreference: [Destructors](https://en.cppreference.com/w/cpp/language/destructor)
- Python documentation: [Data model - `__new__` and `__init__`](https://docs.python.org/3/reference/datamodel.html#basic-customization)
- Python documentation: [dataclasses](https://docs.python.org/3/library/dataclasses.html)
- Python documentation: [The `with` statement](https://docs.python.org/3/reference/compound_stmts.html#the-with-statement)
