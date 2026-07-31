#include "src/Utils/Concurrency/ThreadPoolManager.hpp"

#include <cstddef>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<Concurrency::ThreadPoolManager>);
static_assert(!std::is_move_constructible_v<Concurrency::ThreadPoolManager>);
static_assert(std::is_same_v<
    decltype(std::declval<const Concurrency::ThreadPoolManager&>().thread_count()),
    std::size_t>);

int main()
{
    Concurrency::ThreadPoolManager pool(1);
    return pool.thread_count() == 1 ? 0 : 1;
}
