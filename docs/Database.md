# Device Database Storage

The database storage facade provides a small API for storing network devices without depending directly on a specific database engine. Application code can use `IDatabase`, while concrete implementations such as `SQLiteDatabase` hide SQL and connection details.

## Files

- `src/Utils/Storage/DatabaseItem.hpp`: item stored by the facade.
- `src/Utils/Storage/IDatabase.hpp`: virtual database API used by application code.
- `src/Utils/Storage/SQLiteDatabase.hpp/.cpp`: working SQLite implementation.
- `src/Utils/Storage/MySQLDatabase.hpp/.cpp`: example MySQL-shaped implementation placeholder.

## Stored Item

```cpp
struct DatabaseItem {
    std::string id;
    std::string name;
    std::string ipAddress;
    std::uint16_t port{};
    bool alive{};
};
```

`id` is the unique identifier used by methods such as `GetItem`, `ContainsItem`, and `RemoveItem`.

## Basic Usage

```cpp
Utils::Storage::SQLiteDatabase database("devices.sqlite");
database.Initialize();

database.AddItem({"router", "Main router", "192.168.1.1", 80, true});
database.UpsertItem({"printer-office", "Office printer", "192.168.1.75", 9100, false});

if (const auto router = database.GetItem("router")) {
    // router->ipAddress == "192.168.1.1"
}
```

## API Notes

- `AddItem` inserts a new device and fails if the id already exists.
- `UpsertItem` means update-or-insert: it inserts a missing id or updates the fields for an existing id.
- `GetItem` returns `std::optional<DatabaseItem>` so missing ids can be handled without exceptions.
- `ReplaceAll` clears existing data and inserts the supplied collection in one transaction.
- `GetAllSortedById` returns devices ordered by id.
- `Clear` removes all devices but leaves the database table ready for reuse.

## SQLite Behavior

`SQLiteDatabase` stores devices in a table named `items` with these columns:

- `device_id`
- `name`
- `ip_address`
- `port`
- `alive`

It uses SQLite transactions for `ReplaceAll`, so replacement is committed as one logical operation.

SQLite is a good local embedded database. Multiple processes can read from the same database file, but this facade is currently intended for simple local application storage rather than multi-device synchronization.

## MySQL Placeholder

`MySQLDatabase` currently documents the shape a server-backed implementation would have, but it intentionally throws until a real MySQL client library is added. This keeps the facade example compile-safe without adding a dependency that the application does not use yet.
