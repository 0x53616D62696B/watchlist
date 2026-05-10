#include <filesystem>
#include <format>
#include <vector>

#include "src/Watchlist/SQLiteThreadWorkerExample.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Concurrency {

void run_sqlitecpp_thread_worker_example()
{
    PROFILE_FUNCTION;

    const auto databasePath = std::filesystem::current_path() / "thread_worker_3_storage.sqlite";
    LOG_INFO(std::format("Thread worker 3 opening SQLite database: {}", databasePath.string()));

    Utils::Storage::SQLiteDatabase database(databasePath);
    database.Initialize();

    const std::vector<Utils::Storage::DatabaseItem> items{
        {"refresh_interval_seconds", "60"},
        {"theme", "dark"},
        {"window_layout", "default"},
    };

    {
        PROFILE_SCOPE(SQLiteWorkerInsertTransaction);
        database.ReplaceAll(items);
        database.UpsertItem({"theme", "light"});
        database.AddItem({"last_opened_view", "overview"});
    }

    {
        PROFILE_SCOPE(SQLiteWorkerReadRows);
        for (const auto& item : database.GetAllSortedByKey()) {
            LOG_INFO(std::format("Thread worker 3 database row: {} - {}", item.key, item.value));
        }

        if (const auto theme = database.GetItem("theme")) {
            LOG_INFO(std::format("Thread worker 3 theme setting: {}", theme->value));
        }

        LOG_INFO(std::format("Thread worker 3 database row count: {}", database.CountItems()));
    }
}

} // namespace Concurrency
