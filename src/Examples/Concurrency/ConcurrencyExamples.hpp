#pragma once

namespace Examples::Concurrency {

void RunConcurrencyExamples();

namespace ThreadPoolManager {
void example_thread_pool_manager();
} // namespace ThreadPoolManager

namespace EloopGen {
int example_eloop_gen();
} // namespace EloopGen

namespace EloopCoro {
int example_eloop_coro();
} // namespace EloopCoro

namespace AsyncEloop {
int example_async_eloop();
} // namespace AsyncEloop

} // namespace Examples::Concurrency
