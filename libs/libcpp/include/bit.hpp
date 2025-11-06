#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_

#include <bits_ext.hpp>
#include <concepts.hpp>
#include <limits.hpp>
#include <type_traits.hpp>

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
    return countl_one_constexpr(static_cast<T>(~x));
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
    return countr_one_constexpr(static_cast<T>(~x));
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
        if consteval {                                 \
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
    if consteval {
        return internal::popcount_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::popcount_constexpr(x);
    }
}

// ------------------------------
// bit_width
// ------------------------------

template <unsigned_integral T>
constexpr FAST_CALL int bit_width(const T x) noexcept
{
    return std::numeric_limits<T>::digits - std::countl_zero(x);
}

// ------------------------------
// bit_floor
// ------------------------------

template <std::unsigned_integral T>
constexpr FAST_CALL T bit_floor(T x) noexcept
{
    if (x != 0) {
        return kLsb<T> << (std::bit_width(x) - 1);
    }

    return 0;
}

// ------------------------------
// bit_ceil
// ------------------------------

template <std::unsigned_integral T>
constexpr FAST_CALL T bit_ceil(T x) noexcept
{
    TODO_LIBCPP_COMPLIANCE
    // TODO: Implement code for overflows according to standard

    if (x <= static_cast<T>(1)) {
        return kLsb<T>;
    }

    return kLsb<T> << std::bit_width(T(x - static_cast<T>(1)));
}

TODO_LIBCPP_COMPLIANCE
// TODO: Implement other functions from standard

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
