#include <SQLiteCpp/SQLiteCpp.h>

#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include "src/Watchlist/SQLiteThreadWorkerExample.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

namespace {

struct WatchlistInstrument {
    std::string symbol;
    std::string name;
};

} // namespace

void run_sqlitecpp_thread_worker_example()
{
    PROFILE_FUNCTION;

    const auto databasePath = std::filesystem::current_path() / "thread_worker_3_watchlist.sqlite";
    LOG_INFO(std::format("Thread worker 3 opening SQLite database: {}", databasePath.string()));

    SQLite::Database database(databasePath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    database.exec(
        "CREATE TABLE IF NOT EXISTS watchlist ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "symbol TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL"
        ")");

    database.exec("DELETE FROM watchlist");

    const std::vector<WatchlistInstrument> instruments{
        {"AAPL", "Apple Inc."},
        {"MSFT", "Microsoft Corporation"},
        {"NVDA", "NVIDIA Corporation"},
    };

    {
        PROFILE_SCOPE(SQLiteWorkerInsertTransaction);
        SQLite::Transaction transaction(database);
        SQLite::Statement insert(
            database,
            "INSERT INTO watchlist (symbol, name) VALUES (?, ?)");

        for (const auto& instrument : instruments) {
            insert.bind(1, instrument.symbol);
            insert.bind(2, instrument.name);
            insert.exec();
            insert.reset();
        }

        transaction.commit();
    }

    {
        PROFILE_SCOPE(SQLiteWorkerReadRows);
        SQLite::Statement query(
            database,
            "SELECT symbol, name FROM watchlist ORDER BY symbol");

        while (query.executeStep()) {
            const std::string symbol = query.getColumn(0).getString();
            const std::string name = query.getColumn(1).getString();
            LOG_INFO(std::format("Thread worker 3 database row: {} - {}", symbol, name));
        }
    }
}

} // namespace Concurrency
