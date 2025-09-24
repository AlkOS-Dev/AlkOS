#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_BITS_EXT_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_BITS_EXT_HPP_

#include <stdint.h>
#include <extensions/concepts.hpp>
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

template <std::unsigned_integral NumT>
static constexpr NumT kLsb = 1;

template <std::unsigned_integral NumT, u16 Bit>
static constexpr NumT kSingleBit = kLsb<NumT> << Bit;

template <std::unsigned_integral NumT>
static constexpr NumT kMsb = kSingleBit<NumT, sizeof(NumT) * 8 - 1>;

template <std::unsigned_integral NumT>
static constexpr NumT kFullMask = ~static_cast<NumT>(0);

template <std::unsigned_integral NumT, u16 Range>
static constexpr NumT kBitMaskLeft = kFullMask<NumT> << (sizeof(NumT) * 8 - Range);

template <std::unsigned_integral NumT, u16 Range>
static constexpr NumT kBitMaskRight = kFullMask<NumT> >> (sizeof(NumT) * 8 - Range);

template <std::unsigned_integral NumT, const u16 kBitStart, const u16 kBitLength>
static constexpr NumT kBitMask = []() constexpr {
    static_assert(kBitStart < sizeof(NumT) * 8, "bit start overflows given integer type...");
    static_assert(
        kBitStart + kBitLength <= sizeof(NumT) * 8, "bit length overflows given integer type..."
    );

    NumT mask{};
    for (u16 bit = kBitStart; bit < kBitLength; ++bit) {
        mask |= kLsb<NumT> << bit;
    }
    return mask;
}();

template <std::unsigned_integral NumT, const u16 kBitStart, const u16 kBitEnd>
    requires(kBitStart < kBitEnd)
static constexpr NumT kBitMaskRange = kBitMask<NumT, kBitStart, kBitEnd - kBitStart>;

FAST_CALL constexpr bool IsIntegralSize(const size_t size)
{
    return size == 1 || size == 2 || size == 4 || size == 8;
}

template <size_t kSize = 0>
struct UnsignedIntegral {
    static_assert(false, "Provided wrong integral size");
};

template <>
struct UnsignedIntegral<1> {
    using type = u8;
};
template <>
struct UnsignedIntegral<2> {
    using type = u16;
};
template <>
struct UnsignedIntegral<4> {
    using type = u32;
};
template <>
struct UnsignedIntegral<8> {
    using type = u64;
};

// ------------------------------
// Functions
// ------------------------------

template <std::unsigned_integral NumT>
FAST_CALL NumT &SetBit(NumT &num, const u16 bit)
{
    return num |= kLsb<NumT> << bit;
}

template <std::unsigned_integral NumT>
FAST_CALL NumT &ClearBit(NumT &num, const u16 bit)
{
    return num &= ~(kLsb<NumT> << bit);
}

template <std::unsigned_integral NumT>
FAST_CALL NumT &SwitchBit(NumT &num, const u16 bit)
{
    return num ^= kLsb<NumT> << bit;
}

template <std::unsigned_integral NumT>
FAST_CALL NumT &SetBitValue(NumT &num, const u16 bit, const bool val)
{
    return ClearBit(num, bit) |= static_cast<NumT>(val) << bit;
}

template <std::unsigned_integral NumT>
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

template <std::unsigned_integral NumT>
FAST_CALL constexpr NumT AlignUp(const NumT num, const size_t alignment)
{
    if (!std::is_constant_evaluated()) {
        ASSERT_ZERO(alignment % 2);
    }

    return (num + static_cast<NumT>(alignment) - 1) & ~(static_cast<NumT>(alignment) - 1);
}

template <std::unsigned_integral NumT>
FAST_CALL constexpr NumT AlignDown(const NumT num, const size_t alignment)
{
    if (!std::is_constant_evaluated()) {
        ASSERT_ZERO(alignment % 2);
    }

    return num & ~(static_cast<NumT>(alignment) - 1);
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

template <const u16 kBitStart, const u16 kBitLength, std::unsigned_integral NumT>
FAST_CALL constexpr bool AreBitsEnabled(const NumT num)
{
    return (num & kBitMask<NumT, kBitStart, kBitLength>) == kBitMask<NumT, kBitStart, kBitLength>;
}

template <const u16 kBitStart, const u16 kBitEnd, std::unsigned_integral NumT>
FAST_CALL constexpr bool AreBitsEnabledRanged(const NumT num)
{
    return (num & kBitMaskRange<NumT, kBitStart, kBitEnd>) ==
           kBitMaskRange<NumT, kBitStart, kBitEnd>;
}

template <const u16 kBit, std::unsigned_integral NumT>
FAST_CALL constexpr bool IsBitEnabled(const NumT num)
{
    return (num & kSingleBit<NumT, kBit>) == kSingleBit<NumT, kBit>;
}

template <std::unsigned_integral NumT>
FAST_CALL constexpr bool AreBitsEnabled(const NumT num, const NumT mask)
{
    return (num & mask) == mask;
}

template <std::unsigned_integral NumT>
FAST_CALL constexpr bool AreIntersecting(const NumT a, const NumT b)
{
    return (a & b) != 0;
}

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_BITS_EXT_HPP_
