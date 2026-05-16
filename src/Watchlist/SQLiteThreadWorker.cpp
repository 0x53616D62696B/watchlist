#include <filesystem>
#include <format>
#include <vector>

#include "src/Watchlist/SQLiteThreadWorker.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Utils/Storage/DatabaseItem.hpp"
#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Watchlist {

void run_sqlitecpp_thread_worker()
{
    PROFILE_FUNCTION;

    const auto databasePath = std::filesystem::current_path() / "thread_worker_3_storage.sqlite";
    LOG_INFO(std::format("Thread worker 3 opening SQLite database: {}", databasePath.string()));

    Utils::Storage::SQLiteDatabase database(databasePath);
    database.Initialize();

    const std::vector<Utils::Storage::DatabaseItem> devices{
        {"router", "Main router", "192.168.1.1", 80, true},
        {"camera-front-door", "Front door camera", "192.168.1.42", 554, true},
        {"printer-office", "Office printer", "192.168.1.75", 9100, false},
    };

    {
        PROFILE_SCOPE(SQLiteWorkerInsertTransaction);
        database.ReplaceAll(devices);
        database.UpsertItem({
            "printer-office",
            "Office printer",
            "192.168.1.75",
            9100,
            true,
        });
    }

    {
        PROFILE_SCOPE(SQLiteWorkerReadRows);
        for (const auto& device : database.GetAllSortedById()) {
            LOG_INFO(std::format(
                "Thread worker 3 device: {} ({}) at {}:{} alive={}",
                device.name,
                device.id,
                device.ipAddress,
                device.port,
                device.alive));
        }

        if (const auto router = database.GetItem("router")) {
            LOG_INFO(std::format("Thread worker 3 loaded router IP: {}", router->ipAddress));
        }

        LOG_INFO(std::format("Thread worker 3 device count: {}", database.CountItems()));
    }
}

} // namespace Watchlist
