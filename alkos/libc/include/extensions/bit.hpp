#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_

#include <extensions/bits_ext.hpp>
#include <extensions/concepts.hpp>
#include <extensions/limits.hpp>
#include <extensions/type_traits.hpp>

namespace std
{

// ------------------------------
// internal
// ------------------------------

namespace internal
{
template <std::unsigned_integral T>
static constexpr int countl_one_constexpr(const T x)
{
    T iter    = kMsb<T>;
    int count = 0;

    while (AreIntersecting(iter, x)) {
        count++;
        iter >>= 1;
    }

    return count;
}

}  // namespace internal
// ------------------------------
// std namespace
// ------------------------------
template <std::unsigned_integral T>
constexpr FAST_CALL int countl_one(T x) noexcept
{
    if constexpr (std::is_constant_evaluated()) {
        return internal::countl_one_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::countl_one_constexpr(x);
    }
}
}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
