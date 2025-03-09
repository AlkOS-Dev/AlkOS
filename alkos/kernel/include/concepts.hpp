#ifndef KERNEL_INCLUDE_CONCEPTS_HPP_
#define KERNEL_INCLUDE_CONCEPTS_HPP_

#include "extensions/types.hpp"

template <typename T>
struct IsUnsignedInteger {
    static constexpr bool value =
        T(0) < T(-1) && (sizeof(T) == sizeof(u8) || sizeof(T) == sizeof(u16) ||
                         sizeof(T) == sizeof(u32) || sizeof(T) == sizeof(u64));
};

template <typename T>
concept UnsignedIntegerT = IsUnsignedInteger<T>::value;

#endif  // KERNEL_INCLUDE_CONCEPTS_HPP_
