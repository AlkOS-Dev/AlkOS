#ifndef TESTS_HOST_MOCKS_DEFINES_HPP_
#define TESTS_HOST_MOCKS_DEFINES_HPP_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include "types.hpp"

// ------------------------------
// Environment checks
// ------------------------------

#ifdef NDEBUG
static constexpr bool kIsDebugBuild = false;
#else
static constexpr bool kIsDebugBuild = true;
#endif

static constexpr bool kIsKernel = false;

// ------------------------------
// Attributes
// ------------------------------

#define PACK              __attribute__((__packed__))
#define NO_RET            [[noreturn]]
#define FORCE_INLINE_F    inline
#define PREVENT_INLINE    __attribute__((noinline))
#define NODISCARD         [[nodiscard]]
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#define MAYBE_UNUSED      [[maybe_unused]]

#define WRAP_CALL static FORCE_INLINE_F
#define FAST_CALL static FORCE_INLINE_F

// ------------------------------
// Macros
// ------------------------------

#define STRINGIFY(x)   #x
#define TOSTRING(x)    STRINGIFY(x)
#define BARRIER()      std::atomic_thread_fence(std::memory_order_seq_cst)
#define COMPILER_FENCE std::atomic_signal_fence(std::memory_order_seq_cst)

// Map kernel asserts to standard asserts
#undef assert
#define assert(expr)   assert(expr)
#define r_assert(expr) assert(expr)

// ------------------------------
// Literal Operators
// ------------------------------

constexpr u64 Parse(const char *str, const size_t len)
{
    u64 result = 0;
    for (size_t i = 0; i < len; ++i) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

constexpr size_t operator""_size(const char *str, const size_t len)
{
    return static_cast<size_t>(Parse(str, len));
}
constexpr size_t operator""_size(const unsigned long long value)
{
    return static_cast<size_t>(value);
}

constexpr u8 operator""_u8(const char *str, const size_t len)
{
    return static_cast<u8>(Parse(str, len));
}
constexpr u8 operator""_u8(const unsigned long long value) { return static_cast<u8>(value); }

constexpr u16 operator""_u16(const char *str, const size_t len)
{
    return static_cast<u16>(Parse(str, len));
}
constexpr u16 operator""_u16(const unsigned long long value) { return static_cast<u16>(value); }

constexpr u32 operator""_u32(const char *str, const size_t len)
{
    return static_cast<u32>(Parse(str, len));
}
constexpr u32 operator""_u32(const unsigned long long value) { return static_cast<u32>(value); }

constexpr u64 operator""_u64(const char *str, const size_t len)
{
    return static_cast<u64>(Parse(str, len));
}
constexpr u64 operator""_u64(const unsigned long long value) { return static_cast<u64>(value); }

constexpr i8 operator""_i8(const char *str, const size_t len)
{
    return static_cast<i8>(Parse(str, len));
}
constexpr i8 operator""_i8(const unsigned long long value) { return static_cast<i8>(value); }

constexpr i16 operator""_i16(const char *str, const size_t len)
{
    return static_cast<i16>(Parse(str, len));
}
constexpr i16 operator""_i16(const unsigned long long value) { return static_cast<i16>(value); }

constexpr i32 operator""_i32(const char *str, const size_t len)
{
    return static_cast<i32>(Parse(str, len));
}
constexpr i32 operator""_i32(const unsigned long long value) { return static_cast<i32>(value); }

constexpr i64 operator""_i64(const char *str, const size_t len)
{
    return static_cast<i64>(Parse(str, len));
}
constexpr i64 operator""_i64(const unsigned long long value) { return static_cast<i64>(value); }

constexpr byte operator""_byte(const char *str, const size_t len)
{
    return static_cast<byte>(Parse(str, len));
}
constexpr byte operator""_byte(const unsigned long long value) { return static_cast<byte>(value); }

constexpr f32 operator""_f32(const long double value) { return static_cast<f32>(value); }
constexpr f64 operator""_f64(const long double value) { return static_cast<f64>(value); }

// ------------------------------
// Helpers
// ------------------------------

template <size_t ID>
struct empty_t {
};
#define UNIQUE_EMPTY empty_t<__COUNTER__>

#endif  // TESTS_HOST_MOCKS_DEFINES_HPP_
