#pragma once

#include <filesystem>

namespace Watchlist {

void run_sqlitecpp_thread_worker(const std::filesystem::path& databasePath);

} // namespace Watchlist
