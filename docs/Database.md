# Device Database Storage

The database storage facade provides a small API for storing network devices without depending directly on SQLite details. Application code can use `IDatabase`, while the supported `SQLiteDatabase` implementation hides SQL and connection management.

## Files

- `src/Utils/Storage/DatabaseItem.hpp`: item stored by the facade.
- `src/Utils/Storage/IDatabase.hpp`: virtual database API used by application code.
- `src/Utils/Storage/SQLiteDatabase.hpp/.cpp`: working SQLite implementation.

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

Storage owns the domain validation boundary. `AddItem`, `UpsertItem`, and `ReplaceAll` validate every item before changing the database and throw `DatabaseValidationError`, whose `Field()` identifies the invalid field. IDs contain 1-128 valid UTF-8 bytes, names contain 1-256 valid UTF-8 bytes, addresses are IPv4/IPv6 literals or ASCII DNS hostnames up to 253 bytes, ports cover the complete `std::uint16_t` range, and `alive` is a boolean. Duplicate IDs within `ReplaceAll` are rejected before its transaction begins.

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

Production startup resolves the database independently of the working directory: `%LOCALAPPDATA%/Watchlist/watchlist.sqlite` on Windows, `$XDG_DATA_HOME/watchlist/watchlist.sqlite` or `$HOME/.local/share/watchlist/watchlist.sqlite` on Linux, and `$HOME/Library/Application Support/Watchlist/watchlist.sqlite` on macOS. `--database-path <file>` overrides it. Parent directories are created on first use; missing configuration fails without fallback. Startup initializes and reads only and never seeds stored devices.

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

The SQLite schema is version 1, recorded in `PRAGMA user_version`. Initialization validates the exact table, column, type, nullability, primary-key, unique-index, and check-constraint contract before enabling operations. A fresh database creates v1. The exact former unversioned device schema is migrated transactionally through a separate destination table with checked row decoding and row-count verification. The obsolete key/value schema, unknown versions, and partial or unexpected schemas throw `DatabaseSchemaError` without modifying stored data.

All successful records satisfy defensive SQLite checks, including UTF-8 byte-length bounds, `port BETWEEN 0 AND 65535`, and `alive IN (0,1)`. All query paths use one canonical projection and checked decoder; externally corrupted numeric or domain data throws `DatabaseValidationError` instead of being narrowed or normalized.

SQLite is a good local embedded database. Multiple processes can read from the same database file, but this facade is currently intended for simple local application storage rather than multi-device synchronization.

## Supported Backend

SQLite is the only supported backend. Unsupported server databases are not exposed as constructible implementations; a future backend must provide a working client integration and explicit configuration capability before it is added to the production target.
