#include "src/Examples/Concurrency/ConcurrencyExamples.hpp"

#include <cstdlib>
#include <exception>

#include "src/Utils/Concurrency/AsyncEventLoop.hpp"
#include "src/Utils/Concurrency/EventLoopCoroutine.hpp"
#include "src/Utils/Concurrency/EventLoopGenerator.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Examples::Concurrency {

void RunConcurrencyExamples()
{
    PROFILE_THREAD("Concurrency examples");
    PROFILE_SCOPE(ConcurrencyExamples);

    LOG_INFO("Starting Concurrency examples");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Thread Pool Manager example starting");
    ::Concurrency::example_thread_pool_manager();
    LOG_INFO("thread_pool DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Generator example starting");
    ::Concurrency::example_eloop_gen();
    LOG_INFO("main_eloop_gen DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Coroutine example starting");
    ::Concurrency::example_eloop_coro();
    LOG_INFO("main_eloop_coro DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Async ELoop example starting");
    ::Concurrency::example_async_eloop();
    LOG_INFO("main_eloop_hybrid DONE");
}

} // namespace Examples::Concurrency

int main()
try
{
    Examples::Concurrency::RunConcurrencyExamples();
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    LOG_EXCEPTION(e);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Concurrency examples failed with an unknown error.");
    return EXIT_FAILURE;
}
