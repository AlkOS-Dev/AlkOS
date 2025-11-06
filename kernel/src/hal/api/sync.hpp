#ifndef KERNEL_SRC_HAL_API_SYNC_HPP_
#define KERNEL_SRC_HAL_API_SYNC_HPP_

#include <type_traits.hpp>
#include <types.hpp>

namespace hal
{
struct alignas(4) Atomic32 {
    using BaseT = i32;
    volatile BaseT value;
};

struct alignas(8) Atomic64 {
    using BaseT = i64;
    volatile BaseT value;
};

template <class T>
concept AtomicT = std::is_same_v<T, Atomic32> || std::is_same_v<T, Atomic64>;
}  // namespace hal

#endif  // KERNEL_SRC_HAL_API_SYNC_HPP_
