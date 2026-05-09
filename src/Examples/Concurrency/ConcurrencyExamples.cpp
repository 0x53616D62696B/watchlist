#include "src/Examples/Concurrency/ConcurrencyExamples.hpp"

#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>

#include "src/Utils/Concurrency/AsyncEventLoop.hpp"
#include "src/Utils/Concurrency/EventLoopCoroutine.hpp"
#include "src/Utils/Concurrency/EventLoopGenerator.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Examples::Concurrency {

void RunConcurrencyExamples()
{
    PROFILE_THREAD("Concurrency examples");
    PROFILE_SCOPE(ConcurrencyExamples);

    std::cout << std::format("Starting Concurrency examples") << std::endl;

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Thread Pool Manager example starting");
    ::Concurrency::example_thread_pool_manager();
    std::cout << std::format("thread_pool DONE") << std::endl;

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Generator example starting");
    ::Concurrency::example_eloop_gen();
    std::cout << std::format("main_eloop_gen DONE\n") << std::endl;

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Coroutine example starting");
    ::Concurrency::example_eloop_coro();
    std::cout << std::format("main_eloop_coro DONE\n") << std::endl;

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Async ELoop example starting");
    ::Concurrency::example_async_eloop();
    std::cout << std::format("main_eloop_hybrid DONE\n") << std::endl;
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
    std::cerr << "Concurrency examples failed: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "Concurrency examples failed with an unknown error." << std::endl;
    return EXIT_FAILURE;
}
