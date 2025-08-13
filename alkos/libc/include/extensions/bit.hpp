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
// ------------------------------
// countX implementations
// ------------------------------

template <std::unsigned_integral T>
static constexpr int countl_one_constexpr(const T x) noexcept
{
    if (x == kFullMask<T>) {
        return sizeof(T) * 8;
    }

    T iter    = kMsb<T>;
    int count = 0;

    while (AreIntersecting(iter, x)) {
        count++;
        iter >>= 1;
    }

    return count;
}

template <std::unsigned_integral T>
FAST_CALL constexpr int countl_zero_constexpr(const T x) noexcept
{
    return countl_one_constexpr(~x);
}

template <std::unsigned_integral T>
static constexpr int countr_one_constexpr(const T x) noexcept
{
    if (x == kFullMask<T>) {
        return sizeof(T) * 8;
    }

    T iter    = kLsb<T>;
    int count = 0;

    while (AreIntersecting(iter, x)) {
        count++;
        iter <<= 1;
    }

    return count;
}

template <std::unsigned_integral T>
FAST_CALL constexpr int countr_zero_constexpr(const T x) noexcept
{
    return countr_one_constexpr(~x);
}

// ------------------------------
// popcount implementation
// ------------------------------

template <std::unsigned_integral T>
constexpr int popcount_constexpr(const T x) noexcept
{
    if (x == 0) {
        return 0;
    }

    T iter    = x;
    int count = 0;

    while (iter != 0) {
        count += AreIntersecting(kLsb<T>, iter);
        iter >>= 1;
    }

    return count;
}

}  // namespace internal

// ------------------------------
// countX implementations
// ------------------------------

#define DEFINE_PUBLIC_FUNCTION(func_name)              \
    template <std::unsigned_integral T>                \
    constexpr FAST_CALL int func_name(T x) noexcept    \
    {                                                  \
        if (std::is_constant_evaluated()) {            \
            return internal::func_name##_constexpr(x); \
        } else {                                       \
            TODO_OPTIMISE                              \
            return internal::func_name##_constexpr(x); \
        }                                              \
    }

DEFINE_PUBLIC_FUNCTION(countl_one)
DEFINE_PUBLIC_FUNCTION(countl_zero)
DEFINE_PUBLIC_FUNCTION(countr_one)
DEFINE_PUBLIC_FUNCTION(countr_zero)

// ------------------------------
// popcount implementation
// ------------------------------

template <std::unsigned_integral T>
constexpr FAST_CALL int popcount(T x) noexcept
{
    if (std::is_constant_evaluated()) {
        return internal::popcount_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::popcount_constexpr(x);
    }
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
