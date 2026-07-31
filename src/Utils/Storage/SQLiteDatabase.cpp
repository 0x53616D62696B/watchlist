#include "src/Utils/Storage/SQLiteDatabase.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Utils::Storage {

namespace {

constexpr int CurrentSchemaVersion = 1;
constexpr std::string_view CanonicalProjection = "device_id, name, ip_address, port, alive";
constexpr std::string_view CreateUnversionedItemsSql =
    "CREATE TABLE items ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "device_id TEXT NOT NULL UNIQUE,"
    "name TEXT NOT NULL,"
    "ip_address TEXT NOT NULL,"
    "port INTEGER NOT NULL,"
    "alive INTEGER NOT NULL"
    ")";
constexpr std::string_view CreateLegacyItemsSql =
    "CREATE TABLE items ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "item_key TEXT NOT NULL UNIQUE,"
    "item_value TEXT NOT NULL"
    ")";
constexpr std::string_view CreateV1ItemsSql =
    "CREATE TABLE items ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "device_id TEXT NOT NULL UNIQUE CHECK(typeof(device_id) = 'text' AND length(CAST(device_id AS BLOB)) BETWEEN 1 AND 128),"
    "name TEXT NOT NULL CHECK(typeof(name) = 'text' AND length(CAST(name AS BLOB)) BETWEEN 1 AND 256),"
    "ip_address TEXT NOT NULL CHECK(typeof(ip_address) = 'text' AND length(CAST(ip_address AS BLOB)) BETWEEN 1 AND 253),"
    "port INTEGER NOT NULL CHECK(typeof(port) = 'integer' AND port BETWEEN 0 AND 65535),"
    "alive INTEGER NOT NULL CHECK(typeof(alive) = 'integer' AND alive IN (0, 1))"
    ")";

struct ColumnDefinition {
    std::string name;
    std::string type;
    bool notNull;
    int primaryKey;

    bool operator==(const ColumnDefinition&) const = default;
};

std::vector<ColumnDefinition> ReadColumns(SQLite::Database& database, std::string_view table)
{
    SQLite::Statement query(database, "PRAGMA table_info(" + std::string(table) + ")");
    std::vector<ColumnDefinition> columns;
    while (query.executeStep()) {
        columns.push_back({
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            query.getColumn(3).getInt() != 0,
            query.getColumn(5).getInt(),
        });
    }
    return columns;
}

const std::vector<ColumnDefinition>& DeviceColumns()
{
    static const std::vector<ColumnDefinition> columns{
        {"id", "INTEGER", false, 1},
        {"device_id", "TEXT", true, 0},
        {"name", "TEXT", true, 0},
        {"ip_address", "TEXT", true, 0},
        {"port", "INTEGER", true, 0},
        {"alive", "INTEGER", true, 0},
    };
    return columns;
}

const std::vector<ColumnDefinition>& LegacyKeyValueColumns()
{
    static const std::vector<ColumnDefinition> columns{
        {"id", "INTEGER", false, 1},
        {"item_key", "TEXT", true, 0},
        {"item_value", "TEXT", true, 0},
    };
    return columns;
}

bool TableExists(SQLite::Database& database, std::string_view table)
{
    SQLite::Statement query(database,
        "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1");
    query.bind(1, std::string(table));
    return query.executeStep();
}

std::vector<std::string> ReadUserObjects(SQLite::Database& database)
{
    SQLite::Statement query(database,
        "SELECT type || ':' || name FROM sqlite_master "
        "WHERE name NOT LIKE 'sqlite_%' AND type IN ('table', 'view', 'trigger') ORDER BY 1");
    std::vector<std::string> objects;
    while (query.executeStep()) {
        objects.push_back(query.getColumn(0).getString());
    }
    return objects;
}

std::string ReadCreateSql(SQLite::Database& database, std::string_view table)
{
    SQLite::Statement query(database,
        "SELECT sql FROM sqlite_master WHERE type = 'table' AND name = ?");
    query.bind(1, std::string(table));
    if (!query.executeStep() || query.getColumn(0).isNull()) {
        throw DatabaseSchemaError("items table has no CREATE TABLE definition");
    }
    return query.getColumn(0).getString();
}

std::string NormalizeSql(std::string_view sql)
{
    std::string normalized;
    normalized.reserve(sql.size());
    for (const unsigned char character : sql) {
        if (!std::isspace(character) && character != '"' && character != '`'
            && character != '[' && character != ']' && character != ';') {
            normalized.push_back(static_cast<char>(std::tolower(character)));
        }
    }
    return normalized;
}

void ValidateOnlyItemsObject(SQLite::Database& database)
{
    const auto objects = ReadUserObjects(database);
    if (objects != std::vector<std::string>{"table:items"}) {
        throw DatabaseSchemaError("database contains an unexpected table, view, or trigger");
    }
}

void ValidateUniqueDeviceId(SQLite::Database& database)
{
    SQLite::Statement indexes(database, "PRAGMA index_list(items)");
    int indexCount = 0;
    while (indexes.executeStep()) {
        ++indexCount;
        if (indexes.getColumn(2).getInt() != 1 || indexes.getColumn(4).getInt() != 0) {
            throw DatabaseSchemaError("items has an unexpected non-unique or partial index");
        }

        const auto indexName = indexes.getColumn(1).getString();
        SQLite::Statement columns(database, "PRAGMA index_info('" + indexName + "')");
        if (!columns.executeStep() || columns.getColumn(2).getString() != "device_id"
            || columns.executeStep()) {
            throw DatabaseSchemaError("items unique index must contain only device_id");
        }
    }
    if (indexCount != 1) {
        throw DatabaseSchemaError("items must have exactly one unique device_id index");
    }
}

void ValidateDeviceColumns(SQLite::Database& database)
{
    if (ReadColumns(database, "items") != DeviceColumns()) {
        throw DatabaseSchemaError("items columns, types, nullability, or primary key do not match the device schema");
    }
    ValidateUniqueDeviceId(database);
}

void ValidateExactCreateSql(SQLite::Database& database, std::string_view expected, std::string_view schemaName)
{
    if (NormalizeSql(ReadCreateSql(database, "items")) != NormalizeSql(expected)) {
        throw DatabaseSchemaError("items CREATE TABLE definition does not exactly match " + std::string(schemaName));
    }
}

void ValidateV1Schema(SQLite::Database& database)
{
    ValidateOnlyItemsObject(database);
    ValidateDeviceColumns(database);
    ValidateExactCreateSql(database, CreateV1ItemsSql, "schema v1");
}

bool IsValidUtf8(std::string_view value)
{
    for (std::size_t index = 0; index < value.size();) {
        const auto lead = static_cast<unsigned char>(value[index]);
        std::size_t continuationCount = 0;
        std::uint32_t codePoint = 0;
        if (lead <= 0x7f) {
            ++index;
            continue;
        }
        if ((lead & 0xe0) == 0xc0) {
            continuationCount = 1;
            codePoint = lead & 0x1f;
        } else if ((lead & 0xf0) == 0xe0) {
            continuationCount = 2;
            codePoint = lead & 0x0f;
        } else if ((lead & 0xf8) == 0xf0) {
            continuationCount = 3;
            codePoint = lead & 0x07;
        } else {
            return false;
        }
        if (index + continuationCount >= value.size()) {
            return false;
        }
        for (std::size_t offset = 1; offset <= continuationCount; ++offset) {
            const auto continuation = static_cast<unsigned char>(value[index + offset]);
            if ((continuation & 0xc0) != 0x80) {
                return false;
            }
            codePoint = (codePoint << 6) | (continuation & 0x3f);
        }
        const std::uint32_t minimum = continuationCount == 1 ? 0x80
            : continuationCount == 2                         ? 0x800
                                                             : 0x10000;
        if (codePoint < minimum || codePoint > 0x10ffff
            || (codePoint >= 0xd800 && codePoint <= 0xdfff)) {
            return false;
        }
        index += continuationCount + 1;
    }
    return true;
}

bool IsValidIpv4(std::string_view address)
{
    int parts = 0;
    std::size_t begin = 0;
    while (begin <= address.size()) {
        const auto end = address.find('.', begin);
        const auto part = address.substr(begin,
            end == std::string_view::npos ? address.size() - begin : end - begin);
        if (part.empty() || part.size() > 3
            || !std::ranges::all_of(part, [](unsigned char c) { return std::isdigit(c) != 0; })) {
            return false;
        }
        int value = 0;
        for (const char digit : part) {
            value = value * 10 + (digit - '0');
        }
        if (value > 255) {
            return false;
        }
        ++parts;
        if (end == std::string_view::npos) {
            break;
        }
        begin = end + 1;
    }
    return parts == 4;
}

int CountIpv6Parts(std::string_view sequence, bool allowIpv4)
{
    if (sequence.empty()) {
        return 0;
    }
    int count = 0;
    std::size_t begin = 0;
    while (begin <= sequence.size()) {
        const auto end = sequence.find(':', begin);
        const auto part = sequence.substr(begin,
            end == std::string_view::npos ? sequence.size() - begin : end - begin);
        if (part.empty()) {
            return -1;
        }
        if (part.find('.') != std::string_view::npos) {
            if (!allowIpv4 || end != std::string_view::npos || !IsValidIpv4(part)) {
                return -1;
            }
            count += 2;
        } else {
            if (part.size() > 4 || !std::ranges::all_of(part, [](unsigned char c) {
                    return std::isxdigit(c) != 0;
                })) {
                return -1;
            }
            ++count;
        }
        if (end == std::string_view::npos) {
            break;
        }
        begin = end + 1;
    }
    return count;
}

bool IsValidIpv6(std::string_view address)
{
    const auto compression = address.find("::");
    if (compression != std::string_view::npos
        && address.find("::", compression + 2) != std::string_view::npos) {
        return false;
    }
    if (compression == std::string_view::npos) {
        return CountIpv6Parts(address, true) == 8;
    }
    const auto left = CountIpv6Parts(address.substr(0, compression), false);
    const auto right = CountIpv6Parts(address.substr(compression + 2), true);
    return left >= 0 && right >= 0 && left + right < 8;
}

bool IsValidHostname(std::string_view hostname)
{
    if (hostname.empty() || hostname.size() > 253) {
        return false;
    }
    if (hostname.back() == '.') {
        hostname.remove_suffix(1);
    }
    if (hostname.empty()) {
        return false;
    }
    const bool numericDotsOnly = std::ranges::all_of(hostname, [](unsigned char c) {
        return std::isdigit(c) != 0 || c == '.';
    });
    if (numericDotsOnly) {
        return false;
    }

    std::size_t begin = 0;
    while (begin <= hostname.size()) {
        const auto end = hostname.find('.', begin);
        const auto label = hostname.substr(begin,
            end == std::string_view::npos ? hostname.size() - begin : end - begin);
        if (label.empty() || label.size() > 63
            || !std::isalnum(static_cast<unsigned char>(label.front()))
            || !std::isalnum(static_cast<unsigned char>(label.back()))) {
            return false;
        }
        if (!std::ranges::all_of(label, [](unsigned char c) {
                return std::isalnum(c) != 0 || c == '-';
            })) {
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        begin = end + 1;
    }
    return true;
}

void ValidateSizedUtf8(std::string_view value, std::size_t maximum,
    DatabaseField field, std::string_view fieldName)
{
    if (value.empty() || value.size() > maximum || !IsValidUtf8(value)) {
        throw DatabaseValidationError(field,
            std::string(fieldName) + " must contain 1-" + std::to_string(maximum) + " valid UTF-8 bytes");
    }
}

void ValidateItem(const DatabaseItem& item)
{
    ValidateSizedUtf8(item.id, 128, DatabaseField::Id, "device id");
    ValidateSizedUtf8(item.name, 256, DatabaseField::Name, "device name");
    if (!(IsValidIpv4(item.ipAddress) || IsValidIpv6(item.ipAddress)
            || IsValidHostname(item.ipAddress))) {
        throw DatabaseValidationError(DatabaseField::Address,
            "address must be an IPv4/IPv6 literal or ASCII DNS hostname of at most 253 bytes");
    }
}

void ValidateItems(const std::vector<DatabaseItem>& items)
{
    std::set<std::string> ids;
    for (const auto& item : items) {
        ValidateItem(item);
        if (!ids.insert(item.id).second) {
            throw DatabaseValidationError(DatabaseField::Collection,
                "replacement contains duplicate device id: " + item.id);
        }
    }
}

DatabaseItem DecodeDatabaseItem(SQLite::Statement& query)
{
    for (int index = 0; index < 3; ++index) {
        if (!query.getColumn(index).isText()) {
            throw DatabaseValidationError(
                index == 0 ? DatabaseField::Id : index == 1 ? DatabaseField::Name : DatabaseField::Address,
                "persisted device text field has an invalid SQLite type");
        }
    }
    if (!query.getColumn(3).isInteger()) {
        throw DatabaseValidationError(DatabaseField::Port, "persisted port is not an integer");
    }
    if (!query.getColumn(4).isInteger()) {
        throw DatabaseValidationError(DatabaseField::Alive, "persisted alive value is not an integer");
    }

    const auto port = query.getColumn(3).getInt64();
    if (port < 0 || port > std::numeric_limits<std::uint16_t>::max()) {
        throw DatabaseValidationError(DatabaseField::Port, "persisted port is outside 0-65535");
    }
    const auto alive = query.getColumn(4).getInt64();
    if (alive != 0 && alive != 1) {
        throw DatabaseValidationError(DatabaseField::Alive, "persisted alive value is not 0 or 1");
    }

    DatabaseItem item{
        query.getColumn(0).getString(),
        query.getColumn(1).getString(),
        query.getColumn(2).getString(),
        static_cast<std::uint16_t>(port),
        alive == 1,
    };
    ValidateItem(item);
    return item;
}

void BindItem(SQLite::Statement& statement, const DatabaseItem& item)
{
    statement.bind(1, item.id);
    statement.bind(2, item.name);
    statement.bind(3, item.ipAddress);
    statement.bind(4, static_cast<int>(item.port));
    statement.bind(5, item.alive ? 1 : 0);
}

void InsertItem(SQLite::Database& database, std::string_view table, const DatabaseItem& item)
{
    SQLite::Statement insert(database,
        "INSERT INTO " + std::string(table)
            + " (device_id, name, ip_address, port, alive) VALUES (?, ?, ?, ?, ?)");
    BindItem(insert, item);
    insert.exec();
}

void MigrateUnversionedDeviceSchema(SQLite::Database& database)
{
    ValidateOnlyItemsObject(database);
    ValidateDeviceColumns(database);
    ValidateExactCreateSql(database, CreateUnversionedItemsSql, "the supported unversioned device schema");

    SQLite::Transaction transaction(database);
    std::vector<DatabaseItem> items;
    {
        SQLite::Statement source(database,
            "SELECT " + std::string(CanonicalProjection) + " FROM items ORDER BY id");
        while (source.executeStep()) {
            items.push_back(DecodeDatabaseItem(source));
        }
    }

    database.exec(std::string(CreateV1ItemsSql).replace(
        std::string(CreateV1ItemsSql).find("items"), 5, "items_v1"));
    for (const auto& item : items) {
        InsertItem(database, "items_v1", item);
    }

    {
        SQLite::Statement count(database, "SELECT COUNT(*) FROM items_v1");
        count.executeStep();
        if (count.getColumn(0).getInt64() != static_cast<std::int64_t>(items.size())) {
            throw DatabaseSchemaError("v1 migration row-count verification failed");
        }
    }

    database.exec("DROP TABLE items");
    database.exec("ALTER TABLE items_v1 RENAME TO items");
    database.exec("PRAGMA user_version = 1");
    ValidateV1Schema(database);
    transaction.commit();
}

} // namespace

SQLiteDatabase::SQLiteDatabase(const std::filesystem::path& databasePath)
    : database_(databasePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
}

void SQLiteDatabase::Initialize()
{
    initialized_ = false;
    const auto version = database_.execAndGet("PRAGMA user_version").getInt();
    if (version == CurrentSchemaVersion) {
        ValidateV1Schema(database_);
        initialized_ = true;
        return;
    }
    if (version != 0) {
        throw DatabaseSchemaError("unsupported database schema version: " + std::to_string(version));
    }

    if (!TableExists(database_, "items")) {
        if (!ReadUserObjects(database_).empty()) {
            throw DatabaseSchemaError("unversioned database contains unexpected objects");
        }
        SQLite::Transaction transaction(database_);
        database_.exec(std::string(CreateV1ItemsSql));
        database_.exec("PRAGMA user_version = 1");
        ValidateV1Schema(database_);
        transaction.commit();
        initialized_ = true;
        return;
    }

    if (ReadColumns(database_, "items") == LegacyKeyValueColumns()
        && NormalizeSql(ReadCreateSql(database_, "items")) == NormalizeSql(CreateLegacyItemsSql)) {
        throw DatabaseSchemaError("legacy key/value items schema is unsupported and was left unchanged");
    }

    MigrateUnversionedDeviceSchema(database_);
    initialized_ = true;
}

void SQLiteDatabase::EnsureInitialized() const
{
    if (!initialized_) {
        throw std::logic_error("SQLiteDatabase::Initialize must succeed before storage operations");
    }
}

void SQLiteDatabase::AddItem(const DatabaseItem& item)
{
    EnsureInitialized();
    ValidateItem(item);
    InsertItem(database_, "items", item);
}

void SQLiteDatabase::UpsertItem(const DatabaseItem& item)
{
    EnsureInitialized();
    ValidateItem(item);
    SQLite::Statement upsert(database_,
        "INSERT INTO items (device_id, name, ip_address, port, alive) VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(device_id) DO UPDATE SET name=excluded.name, ip_address=excluded.ip_address, "
        "port=excluded.port, alive=excluded.alive");
    BindItem(upsert, item);
    upsert.exec();
}

std::optional<DatabaseItem> SQLiteDatabase::GetItem(const std::string& id) const
{
    EnsureInitialized();
    SQLite::Statement query(QueryConnection(),
        "SELECT " + std::string(CanonicalProjection) + " FROM items WHERE device_id = ?");
    query.bind(1, id);
    if (!query.executeStep()) {
        return std::nullopt;
    }
    return DecodeDatabaseItem(query);
}

std::vector<DatabaseItem> SQLiteDatabase::GetAllItems() const
{
    EnsureInitialized();
    SQLite::Statement query(QueryConnection(),
        "SELECT " + std::string(CanonicalProjection) + " FROM items");
    std::vector<DatabaseItem> items;
    while (query.executeStep()) {
        items.push_back(DecodeDatabaseItem(query));
    }
    return items;
}

void SQLiteDatabase::ReplaceAll(const std::vector<DatabaseItem>& items)
{
    EnsureInitialized();
    ValidateItems(items);
    SQLite::Transaction transaction(database_);
    database_.exec("DELETE FROM items");
    for (const auto& item : items) {
        InsertItem(database_, "items", item);
    }
    transaction.commit();
}

std::vector<DatabaseItem> SQLiteDatabase::GetAllSortedById() const
{
    EnsureInitialized();
    SQLite::Statement query(QueryConnection(),
        "SELECT " + std::string(CanonicalProjection) + " FROM items ORDER BY device_id");
    std::vector<DatabaseItem> items;
    while (query.executeStep()) {
        items.push_back(DecodeDatabaseItem(query));
    }
    return items;
}

bool SQLiteDatabase::ContainsItem(const std::string& id) const
{
    EnsureInitialized();
    SQLite::Statement query(QueryConnection(), "SELECT 1 FROM items WHERE device_id = ? LIMIT 1");
    query.bind(1, id);
    return query.executeStep();
}

std::int64_t SQLiteDatabase::CountItems() const
{
    EnsureInitialized();
    SQLite::Statement query(QueryConnection(), "SELECT COUNT(*) FROM items");
    query.executeStep();
    return query.getColumn(0).getInt64();
}

bool SQLiteDatabase::RemoveItem(const std::string& id)
{
    EnsureInitialized();
    SQLite::Statement remove(database_, "DELETE FROM items WHERE device_id = ?");
    remove.bind(1, id);
    return remove.exec() > 0;
}

void SQLiteDatabase::Clear()
{
    EnsureInitialized();
    database_.exec("DELETE FROM items");
}

} // namespace Utils::Storage
