#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_

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

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL constexpr bool IsAligned(const NumT num, const size_t alignment)
{
    return (num & (alignment - 1)) == 0;
}

// Overload for pointer types
template <typename PtrT>
    requires std::is_pointer_v<PtrT>
FAST_CALL constexpr bool IsAligned(const PtrT ptr, const size_t alignment)
{
    return IsAligned(reinterpret_cast<uintptr_t>(ptr), alignment);
}

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL constexpr NumT AlignUp(const NumT num, const size_t alignment)
{
    return (num + alignment - 1) & ~(alignment - 1);
}

template <typename NumT>
    requires std::is_unsigned_v<NumT>
FAST_CALL constexpr NumT AlignDown(const NumT num, const size_t alignment)
{
    return num & ~(alignment - 1);
}

template <typename PtrT>
    requires std::is_pointer_v<PtrT>
FAST_CALL constexpr PtrT AlignUp(const PtrT ptr, const size_t alignment)
{
    return reinterpret_cast<PtrT>(AlignUp(reinterpret_cast<uintptr_t>(ptr), alignment));
}

template <typename PtrT>
    requires std::is_pointer_v<PtrT>
FAST_CALL constexpr PtrT AlignDown(const PtrT ptr, const size_t alignment)
{
    return reinterpret_cast<PtrT>(AlignDown(reinterpret_cast<uintptr_t>(ptr), alignment));
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BIT_HPP_
