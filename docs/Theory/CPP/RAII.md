# RAII In C++

RAII means **Resource Acquisition Is Initialization**.

The core idea is simple: bind a resource to the lifetime of a C++ object.
The object acquires the resource in its constructor and releases it in its destructor.
When the object goes out of scope, cleanup happens automatically.

RAII is one of the most important C++ idioms because it makes cleanup reliable even when a function returns early or throws an exception.

## The Problem RAII Solves

Many resources need explicit cleanup:

- Memory must be freed.
- Files must be closed.
- Mutexes must be unlocked.
- Network sockets must be closed.
- Database connections and statements must be finalized.

Manual cleanup is easy to forget:

```cpp
auto* ptr = new Widget();

if (!ptr->IsValid()) {
    return; // leak: delete is never called
}

delete ptr;
```

The same problem appears with files, locks, and database handles.
Every early return and every exception creates another path where cleanup might be skipped.

RAII moves cleanup into destructors, so C++ scope rules do the work.

## Basic RAII Shape

```cpp
class ResourceOwner {
public:
    ResourceOwner() {
        // acquire resource
    }

    ~ResourceOwner() {
        // release resource
    }
};
```

Usage:

```cpp
void DoWork() {
    ResourceOwner owner;

    // use the resource

} // owner destructor runs here
```

The destructor runs automatically when `owner` leaves scope.

## Standard Library Examples

Most modern C++ standard library types use RAII.

### Memory

```cpp
auto widget = std::make_unique<Widget>();
widget->Update();
```

`std::unique_ptr` owns the `Widget`.
When `widget` goes out of scope, the `Widget` is deleted automatically.

Prefer this over raw `new` and `delete`.

### Files

```cpp
std::ofstream file("watchlist.txt");
file << "AAPL\n";
```

`std::ofstream` opens the file when constructed and closes it when destroyed.

### Locks

```cpp
std::mutex mutex;

void UpdateState() {
    std::lock_guard<std::mutex> lock(mutex);

    // protected work

} // mutex is unlocked here
```

`std::lock_guard` locks the mutex in its constructor and unlocks it in its destructor.
This prevents bugs where a mutex stays locked after an early return.

## RAII And Databases

Database libraries often use RAII for connections, statements, transactions, and result objects.

Example shape:

```cpp
void LoadWatchlist() {
    DatabaseConnection db("watchlist.db");
    Statement statement(db, "SELECT symbol, name FROM instruments");

    while (statement.Step()) {
        // read row
    }

} // statement is finalized, db connection is closed
```

The important part is not the exact class names.
The important part is ownership:

- `db` owns the database connection.
- `statement` owns the prepared statement.
- Their destructors release the database resources.

This is why C++ wrappers around C libraries can feel much safer.
The raw C API may require calls such as `close`, `free`, or `finalize`.
The C++ wrapper can put those calls in destructors.

## Constructor And Destructor Responsibilities

In RAII:

- Constructors should create a valid object or fail.
- Destructors should release owned resources.
- Ownership should be clear from the type.

A good RAII object should not represent a half-acquired resource unless the type explicitly models that state.

For example, this is a good pattern:

```cpp
class FileHandle {
public:
    explicit FileHandle(const std::filesystem::path& path) {
        // open file or throw on failure
    }

    ~FileHandle() {
        // close file if open
    }
};
```

After construction succeeds, users can trust that `FileHandle` owns a valid file handle.

## Move Semantics

Many RAII types should be movable but not copyable.

Copying an owning object can be dangerous because two objects might try to release the same resource.
Moving transfers ownership from one object to another.

```cpp
std::unique_ptr<Widget> CreateWidget() {
    return std::make_unique<Widget>();
}

auto widget = CreateWidget(); // ownership moved to widget
```

For custom RAII types, a common rule is:

- Disable copying for exclusive ownership.
- Allow moving if ownership transfer makes sense.

```cpp
class Connection {
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&) noexcept = default;
    Connection& operator=(Connection&&) noexcept = default;
};
```

## RAII And Exceptions

RAII is especially useful when exceptions are possible.

```cpp
void SaveData() {
    std::ofstream file("data.txt");
    std::lock_guard<std::mutex> lock(mutex);

    WriteData(file); // might throw

} // file closes and mutex unlocks even if WriteData throws
```

This is called exception safety.
RAII makes cleanup deterministic because destructors run during stack unwinding.

## Practical Rules

- Prefer stack objects over manual heap allocation.
- Prefer `std::unique_ptr` or `std::shared_ptr` over raw owning pointers.
- Prefer standard RAII wrappers such as `std::vector`, `std::string`, `std::ofstream`, and `std::lock_guard`.
- Avoid owning raw pointers in application code.
- If a type owns a resource, make its destructor release that resource.
- Make ownership visible in the type name or API.
- Avoid calling cleanup methods manually when scope-based cleanup is enough.

## Common Mistake

RAII does not mean every class must allocate something in its constructor.
It means that if a class owns a resource, that ownership should be tied to object lifetime.

This is RAII:

```cpp
std::vector<int> values;
values.push_back(42);
```

The vector manages dynamic memory internally.
User code does not call `malloc`, `free`, `new`, or `delete`.

## Short Summary

RAII lets C++ objects own resources safely.
Acquire in the constructor, release in the destructor, and let scope control cleanup.

It makes code easier to reason about because resource lifetime follows object lifetime.
