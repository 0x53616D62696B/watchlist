#include <filesystem>
#include <format>

#include "src/Watchlist/SQLiteThreadWorker.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Utils/Storage/DatabaseItem.hpp"
#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Watchlist {

void run_sqlitecpp_thread_worker(const std::filesystem::path& databasePath)
{
    PROFILE_FUNCTION;

    Utils::Storage::SQLiteDatabase database(databasePath);
    database.Initialize();

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

        LOG_INFO(std::format("Thread worker 3 device count: {}", database.CountItems()));
    }
}

} // namespace Watchlist
