# Values and References in C++

This guide explains **lvalues**, **rvalues**, **lvalue references**, and **rvalue references**.
These ideas matter when deciding whether a function should copy an object, modify an existing object, only read it, or take ownership of its resources.

## Quick Idea

| Concept | Example | Meaning |
| --- | --- | --- |
| lvalue | `lion` | A value with identity, usually something named or addressable |
| rvalue | `Lion{}` | A temporary value, usually something short-lived |
| lvalue reference | `Lion&` | Refers to an existing object that can be modified |
| const lvalue reference | `const Lion&` | Refers to an existing object or temporary without modifying it |
| rvalue reference | `Lion&&` | Refers to a temporary or explicitly movable object |

## Lvalue

An **lvalue** is an expression that refers to an object with identity.
It is usually something that has a name, a stable location, or an address you can take.

```cpp
Lion simba;

simba;     // lvalue
&simba;    // OK, simba has an address
```

Common lvalue examples:

```cpp
simba
animals[0]
person.name
*ptr
```

The old memory trick is that an lvalue can often appear on the left side of assignment:

```cpp
int x = 10;
x = 20; // x is an lvalue
```

But the real rule is about identity, not the literal side of `=`.
In this example, `x` is still an lvalue even though it appears on the right side:

```cpp
int y = x;
```

## Rvalue

An **rvalue** is usually a temporary value.
It does not have a stable name in your code and is often used only for the current expression.

```cpp
Lion{};       // rvalue: temporary Lion
make_lion();  // rvalue if the function returns Lion by value
```

Common rvalue examples:

```cpp
42
x + y
Lion{}
std::string("hello")
```

An rvalue usually cannot appear on the left side of assignment:

```cpp
10 = x; // Error
```

## Lvalue Reference

An **lvalue reference** uses `&`.
It binds to an existing lvalue object.

```cpp
void function(Lion& lion);
```

Example:

```cpp
Lion simba;

function(simba); // OK
function(Lion{}); // Error: temporary cannot bind to non-const Lion&
```

Use `Lion&` when the function should work with the caller's existing object and may modify it.

## Const Lvalue Reference

A **const lvalue reference** uses `const T&`.
It can bind to both lvalues and rvalues, but the function cannot modify the object through that reference.

```cpp
void function(const Lion& lion);
```

Example:

```cpp
Lion simba;

function(simba); // OK: existing object
function(Lion{}); // OK: temporary object
```

Use `const Lion&` when the function only needs to read the object and copying would be unnecessary or expensive.

## Rvalue Reference

An **rvalue reference** uses `&&`.
It binds to temporary values and to objects explicitly marked as movable with `std::move`.

```cpp
void function(Lion&& lion);
```

Example:

```cpp
Lion simba;

function(Lion{}); // OK: temporary
function(std::move(simba)); // OK: simba is treated as movable
```

Use `Lion&&` when the function may move from the object.
Moving can reuse resources from the source object instead of copying them.

After an object is moved from, it must still be valid, but its exact value should not be relied on unless the type documents it.

## Function Parameter Choices

These overloads communicate different ownership and modification intentions:

```cpp
// I will take a local copy of your lion.
// You can keep yours
void function(Lion lion);

// I will take and modify your lion,
// but it is still yours after I am done.
void function(Lion& lion);

// I will just take a look at your lion,
// nothing will be done to it!
void function(const Lion& lion);

// Nice lion, I will take it.
void function(Lion&& lion);
```

### Pass by Value

```cpp
void function(Lion lion);
```

The function receives its own local `Lion`.
For lvalue arguments, this usually means a copy.
For rvalue arguments, this may use a move or be optimized by the compiler.

```cpp
Lion simba;

function(simba);  // copies simba into the parameter
function(Lion{}); // passes a temporary
```

Use pass by value when the function needs its own copy, or when it will store the value and moving from the parameter is useful.

### Pass by Lvalue Reference

```cpp
void function(Lion& lion);
```

The function receives access to the caller's original object.
Changes made through `lion` affect the caller's object.

```cpp
Lion simba;
function(simba); // function can modify simba
```

Use this when modification is intentional and part of the function contract.

### Pass by Const Lvalue Reference

```cpp
void function(const Lion& lion);
```

The function receives read-only access.
No copy is made, and both existing objects and temporaries can be passed.

```cpp
Lion simba;

function(simba);  // reads simba
function(Lion{}); // reads a temporary
```

This is a common choice for large objects that only need to be inspected.

### Pass by Rvalue Reference

```cpp
void function(Lion&& lion);
```

The function receives something that the caller is willing to give up.
This usually means the function may move from it.

```cpp
Lion simba;

function(Lion{});          // temporary can be taken
function(std::move(simba)); // caller explicitly allows moving from simba
```

Important detail: inside the function, the parameter `lion` has a name, so the expression `lion` is an lvalue.
If you want to move from it inside the function, use `std::move`.

```cpp
void function(Lion&& lion) {
    Lion local = std::move(lion);
}
```

## `std::move`

`std::move` does not move anything by itself.
It casts an expression to an rvalue, which allows move constructors or move assignment operators to be used.

```cpp
Lion simba;
Lion nala = std::move(simba);
```

This says: "It is okay to treat `simba` as something that can be moved from."

## Practical Rules

| Function needs to... | Prefer |
| --- | --- |
| Make its own independent object | `Lion lion` |
| Modify the caller's existing object | `Lion& lion` |
| Only read without copying | `const Lion& lion` |
| Take or move from a temporary/movable object | `Lion&& lion` |

When in doubt, start with the clearest ownership rule:

- Use `const T&` to read a large object.
- Use `T&` to modify the caller's object.
- Use `T` when the function needs its own copy.
- Use `T&&` when move semantics are part of the interface.
