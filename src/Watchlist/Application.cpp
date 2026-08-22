#include "src/Watchlist/Application.hpp"

#include <cstdlib>
#include <exception>
#include <stdexcept>

#include "src/Utils/Logger/Logger.hpp"

namespace Watchlist {
namespace {

bool HasArgument(std::span<const std::string_view> arguments, std::string_view argument)
{
    for (const auto candidate : arguments)
    {
        if (candidate == argument)
            return true;
    }
    return false;
}

void ValidateDependencies(const ApplicationDependencies& dependencies)
{
    if (!dependencies.announceStartup || !dependencies.waitForProfiler ||
        !dependencies.resolveDatabasePath || !dependencies.runGui)
    {
        throw std::invalid_argument("application dependencies must all be configured");
    }
}

} // namespace

int RunApplication(
    std::span<const std::string_view> arguments, const ApplicationDependencies& dependencies) noexcept
try
{
    ValidateDependencies(dependencies);
    if (HasArgument(arguments, "--wait-for-tracy"))
        dependencies.waitForProfiler();

    dependencies.announceStartup();
    const auto databasePath = dependencies.resolveDatabasePath(arguments);
    const auto guiResult = dependencies.runGui(databasePath, HasArgument(arguments, "--gui-smoke"));
    if (!guiResult.success)
    {
        if (!guiResult.error.empty())
            LOG_ERROR(guiResult.error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
catch (const std::exception& exception)
{
    LOG_EXCEPTION(exception);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Unknown error in application startup.");
    return EXIT_FAILURE;
}

} // namespace Watchlist
