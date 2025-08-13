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
FAST_CALL constexpr int countl_zero_constexpr(const T x)
{
    return countl_one_constexpr(~x);
}

template <std::unsigned_integral T>
static constexpr int countr_one_constexpr(const T x)
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
FAST_CALL constexpr int countr_zero_constexpr(const T x)
{
    return countr_one_constexpr(~x);
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

template <std::unsigned_integral T>
constexpr FAST_CALL int countl_zero(T x) noexcept
{
    if constexpr (std::is_constant_evaluated()) {
        return internal::countl_zero_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::countl_zero_constexpr(x);
    }
}

template <std::unsigned_integral T>
constexpr FAST_CALL int countr_one(T x) noexcept
{
    if constexpr (std::is_constant_evaluated()) {
        return internal::countr_one_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::countr_one_constexpr(x);
    }
}

template <std::unsigned_integral T>
constexpr FAST_CALL int countr_zero(T x) noexcept
{
    if constexpr (std::is_constant_evaluated()) {
        return internal::countr_zero_constexpr(x);
    } else {
        TODO_OPTIMISE
        return internal::countr_zero_constexpr(x);
    }
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
