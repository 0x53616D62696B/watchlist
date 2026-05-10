#include "src/Utils/Storage/MySQLDatabase.hpp"

#include <utility>

namespace Utils::Storage {

MySQLDatabase::MySQLDatabase(MySQLConnectionSettings settings)
    : settings_(std::move(settings))
{
}

void MySQLDatabase::Initialize()
{
    ThrowMissingClient();
}

void MySQLDatabase::AddItem(const DatabaseItem& item)
{
    (void)item;
    ThrowMissingClient();
}

void MySQLDatabase::UpsertItem(const DatabaseItem& item)
{
    (void)item;
    ThrowMissingClient();
}

std::optional<DatabaseItem> MySQLDatabase::GetItem(const std::string& key)
{
    (void)key;
    ThrowMissingClient();
}

std::vector<DatabaseItem> MySQLDatabase::GetAllItems()
{
    ThrowMissingClient();
}

void MySQLDatabase::ReplaceAll(const std::vector<DatabaseItem>& items)
{
    (void)items;
    ThrowMissingClient();
}

std::vector<DatabaseItem> MySQLDatabase::GetAllSortedByKey()
{
    ThrowMissingClient();
}

bool MySQLDatabase::ContainsItem(const std::string& key)
{
    (void)key;
    ThrowMissingClient();
}

std::int64_t MySQLDatabase::CountItems()
{
    ThrowMissingClient();
}

bool MySQLDatabase::RemoveItem(const std::string& key)
{
    (void)key;
    ThrowMissingClient();
}

void MySQLDatabase::Clear()
{
    ThrowMissingClient();
}

void MySQLDatabase::ThrowMissingClient()
{
    throw std::logic_error("MySQLDatabase is a facade example. Add a MySQL client library before using it.");
}

} // namespace Utils::Storage
