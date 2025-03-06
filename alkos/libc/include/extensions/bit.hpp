#ifndef KERNEL_INCLUDE_BIT_HPP_
#define KERNEL_INCLUDE_BIT_HPP_

#include <stdint.h>
#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>

// ------------------------------
// Various defines
// ------------------------------

static constexpr u64 kBitMask4  = 0xF;
static constexpr u64 kBitMask8  = 0xFF;
static constexpr u64 kBitMask16 = kBitMask8 | (kBitMask8 << 8);
static constexpr u64 kBitMask32 = kBitMask16 | (kBitMask16 << 16);
static constexpr u64 kBitMask64 = kBitMask32 | (kBitMask32 << 32);

template <typename NumT>
    requires std::is_unsigned_v<NumT>
static constexpr NumT kLsb = 1;

template <typename NumT>
    requires std::is_unsigned_v<NumT>
static constexpr NumT kMsb = kLsb<NumT> << sizeof(NumT) * 8 - 1;

template <typename NumT>
    requires std::is_unsigned_v<NumT>
static constexpr NumT kFullMask = ~static_cast<NumT>(0);

// ------------------------------
// Functions
// ------------------------------

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL NumT &SetBit(NumT &num, const u16 bit)
{
    return num |= kLsb<NumT> << bit;
}

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL NumT &ClearBit(NumT &num, const u16 bit)
{
    return num &= ~(kLsb<NumT> << bit);
}

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL NumT &SwitchBit(NumT &num, const u16 bit)
{
    return num ^= kLsb<NumT> << bit;
}

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL NumT &SetBitValue(NumT &num, const u16 bit, const bool val)
{
    return ClearBit(num, bit) |= static_cast<NumT>(val) << bit;
}

#endif  // KERNEL_INCLUDE_BIT_HPP_
