# Database Storage

The database storage facade is a small key/value API for code that needs persistence without depending directly on a specific database engine. Application code can use `IDatabase`, while concrete implementations such as `SQLiteDatabase` hide the SQL and connection details.

## Files

- `src/Utils/Storage/DatabaseItem.hpp`: generic key/value item stored by the facade.
- `src/Utils/Storage/IDatabase.hpp`: virtual database API used by application code.
- `src/Utils/Storage/SQLiteDatabase.hpp/.cpp`: working SQLite implementation.
- `src/Utils/Storage/MySQLDatabase.hpp/.cpp`: example MySQL-shaped implementation placeholder.

## Basic Usage

```cpp
Utils::Storage::SQLiteDatabase database("settings.sqlite");
database.Initialize();

database.AddItem({"theme", "dark"});
database.UpsertItem({"theme", "light"});

if (const auto item = database.GetItem("theme")) {
    // item->value == "light"
}
```

## API Notes

- `AddItem` inserts a new item and fails if the key already exists.
- `UpsertItem` means update-or-insert: it inserts a missing key or updates the value for an existing key.
- `GetItem` returns `std::optional<DatabaseItem>` so missing keys can be handled without exceptions.
- `ReplaceAll` clears existing data and inserts the supplied collection in one transaction.
- `Clear` removes all items but leaves the database table ready for reuse.

## SQLite Behavior

`SQLiteDatabase` stores items in a table named `items` with a unique `item_key` column. It uses SQLite transactions for `ReplaceAll`, so the replacement is committed as one logical operation.

SQLite is a good local embedded database. Multiple processes can read from the same database file, but this facade is currently intended for simple local application storage rather than multi-device synchronization.

## MySQL Placeholder

`MySQLDatabase` currently documents the shape a server-backed implementation would have, but it intentionally throws until a real MySQL client library is added. This keeps the facade example compile-safe without adding a dependency that the application does not use yet.
