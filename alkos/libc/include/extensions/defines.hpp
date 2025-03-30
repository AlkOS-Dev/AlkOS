#ifndef LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
#define LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_

#include <defines.h>
#include <stddef.h>
#include <extensions/types.hpp>

#ifdef NDEBUG
static constexpr bool kIsDebugBuild = false;
#else
static constexpr bool kIsDebugBuild = true;
#endif  // NDEBUG

#ifdef __ALKOS_LIBK__
static constexpr bool kIsKernelBuild = true;
#else
static constexpr bool kIsKernelBuild = false;
#endif  // __ALKOS_LIBK__

#define NODISCARD [[nodiscard]]

constexpr u64 Parse(const char* str, const size_t len)
{
    u64 result = 0;
    for (size_t i = 0; i < len; ++i) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

// ------------------------------
// size_t
// ------------------------------

constexpr size_t operator""_size(const char* str, const size_t len)
{
    return static_cast<size_t>(Parse(str, len));
}

constexpr size_t operator""_size(const unsigned long long value) { return value; }

// ------------------------------
// Unsigned integers
// ------------------------------

constexpr u8 operator""_u8(const char* str, const size_t len)
{
    return static_cast<u8>(Parse(str, len));
}

constexpr u8 operator""_u8(const unsigned long long value) { return static_cast<u8>(value); }

constexpr u16 operator""_u16(const char* str, const size_t len)
{
    return static_cast<u16>(Parse(str, len));
}

constexpr u16 operator""_u16(const unsigned long long value) { return static_cast<u16>(value); }

constexpr u32 operator""_u32(const char* str, const size_t len)
{
    return static_cast<u32>(Parse(str, len));
}

constexpr u32 operator""_u32(const unsigned long long value) { return static_cast<u32>(value); }

constexpr u64 operator""_u64(const char* str, const size_t len)
{
    return static_cast<u64>(Parse(str, len));
}

constexpr u64 operator""_u64(const unsigned long long value) { return static_cast<u64>(value); }

// ------------------------------
// Signed integers
// ------------------------------

constexpr i8 operator""_i8(const char* str, const size_t len)
{
    // Simple implementation, doesn't handle negative numbers in string form
    return static_cast<i8>(Parse(str, len));
}

constexpr i8 operator""_i8(const unsigned long long value) { return static_cast<i8>(value); }

constexpr i16 operator""_i16(const char* str, const size_t len)
{
    return static_cast<i16>(Parse(str, len));
}

constexpr i16 operator""_i16(const unsigned long long value) { return static_cast<i16>(value); }

constexpr i32 operator""_i32(const char* str, const size_t len)
{
    return static_cast<i32>(Parse(str, len));
}

constexpr i32 operator""_i32(const unsigned long long value) { return static_cast<i32>(value); }

constexpr i64 operator""_i64(const char* str, const size_t len)
{
    return static_cast<i64>(Parse(str, len));
}

constexpr i64 operator""_i64(const unsigned long long value) { return static_cast<i64>(value); }

// ------------------------------
// Byte type
// ------------------------------

constexpr byte operator""_byte(const char* str, const size_t len)
{
    return static_cast<byte>(Parse(str, len));
}

constexpr byte operator""_byte(const unsigned long long value) { return static_cast<byte>(value); }

// ------------------------------
// Float types
// ------------------------------

constexpr f32 operator""_f32(const long double value) { return static_cast<f32>(value); }
constexpr f64 operator""_f64(const long double value) { return static_cast<f64>(value); }

#endif  // LIBC_INCLUDE_EXTENSIONS_DEFINES_HPP_
