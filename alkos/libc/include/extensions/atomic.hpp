#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_

#include <assert.h>
#include <stdint.h>
#include <extensions/defines.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/type_traits.hpp>

namespace std
{

// ------------------------------
// memory_order
// ------------------------------

enum class memory_order : i32 {
    relaxed = __ATOMIC_RELAXED,
    consume = __ATOMIC_CONSUME,
    acquire = __ATOMIC_ACQUIRE,
    release = __ATOMIC_RELEASE,
    acq_rel = __ATOMIC_ACQ_REL,
    seq_cst = __ATOMIC_SEQ_CST
};

inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;
inline constexpr memory_order memory_order_consume = memory_order::consume;
inline constexpr memory_order memory_order_acquire = memory_order::acquire;
inline constexpr memory_order memory_order_release = memory_order::release;
inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;
inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;

// ------------------------------
// atomic
// ------------------------------
template <typename T>
    requires std::is_trivially_copyable_v<T> && std::is_copy_constructible_v<T> &&
             std::is_move_constructible_v<T> && std::is_copy_assignable_v<T> &&
             std::is_move_assignable_v<T> && std::is_same_v<T, std::remove_cv_t<T>>
struct atomic : template_lib::NoCopy {
    using value_type                          = T;
    static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(value_type), 0);

    value_type value_;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : value_()
    {
    }

    constexpr atomic(T desired) noexcept : value_(desired) {};

    // ------------------------------
    // operators
    // ------------------------------

    T operator=(T desired) noexcept
    {
        store(desired);
        return desired;
    }
    T operator=(T desired) volatile noexcept
    {
        store(desired);
        return desired;
    }

    // operator T() const noexcept {
    //     return load();
    // }
    // operator T() const volatile noexcept {
    //     return load();
    // }

    // ------------------------------
    // Methods
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool is_lock_free() const noexcept
    {
        return __atomic_is_lock_free(sizeof(value_type), &value_);
    }
    NODISCARD FORCE_INLINE_F bool is_lock_free() const volatile noexcept
    {
        return __atomic_is_lock_free(sizeof(value_type), &value_);
    }
};

// ------------------------------
// aliases
// ------------------------------

using atomic_bool           = atomic<bool>;
using atomic_char           = atomic<char>;
using atomic_schar          = atomic<signed char>;
using atomic_uchar          = atomic<unsigned char>;
using atomic_short          = atomic<short>;
using atomic_ushort         = atomic<unsigned short>;
using atomic_int            = atomic<int>;
using atomic_uint           = atomic<unsigned int>;
using atomic_long           = atomic<long>;
using atomic_ulong          = atomic<unsigned long>;
using atomic_llong          = atomic<long long>;
using atomic_ullong         = atomic<unsigned long long>;
using atomic_char8_t        = atomic<char8_t>;
using atomic_char16_t       = atomic<char16_t>;
using atomic_char32_t       = atomic<char32_t>;
using atomic_wchar_t        = atomic<wchar_t>;
using atomic_int8_t         = atomic<int8_t>;
using atomic_uint8_t        = atomic<uint8_t>;
using atomic_int16_t        = atomic<int16_t>;
using atomic_uint16_t       = atomic<uint16_t>;
using atomic_int32_t        = atomic<int32_t>;
using atomic_uint32_t       = atomic<uint32_t>;
using atomic_int64_t        = atomic<int64_t>;
using atomic_uint64_t       = atomic<uint64_t>;
using atomic_int_least8_t   = atomic<int_least8_t>;
using atomic_uint_least8_t  = atomic<uint_least8_t>;
using atomic_int_least16_t  = atomic<int_least16_t>;
using atomic_uint_least16_t = atomic<uint_least16_t>;
using atomic_int_least32_t  = atomic<int_least32_t>;
using atomic_uint_least32_t = atomic<uint_least32_t>;
using atomic_int_least64_t  = atomic<int_least64_t>;
using atomic_uint_least64_t = atomic<uint_least64_t>;
using atomic_int_fast8_t    = atomic<int_fast8_t>;
using atomic_uint_fast8_t   = atomic<uint_fast8_t>;
using atomic_int_fast16_t   = atomic<int_fast16_t>;
using atomic_uint_fast16_t  = atomic<uint_fast16_t>;
using atomic_int_fast32_t   = atomic<int_fast32_t>;
using atomic_uint_fast32_t  = atomic<uint_fast32_t>;
using atomic_int_fast64_t   = atomic<int_fast64_t>;
using atomic_uint_fast64_t  = atomic<uint_fast64_t>;
using atomic_intptr_t       = atomic<intptr_t>;
using atomic_uintptr_t      = atomic<uintptr_t>;
using atomic_size_t         = atomic<size_t>;
using atomic_ptrdiff_t      = atomic<ptrdiff_t>;
using atomic_intmax_t       = atomic<intmax_t>;
using atomic_uintmax_t      = atomic<uintmax_t>;

// ------------------------------
// atomic_flag
// ------------------------------

class atomic_flag
{
    bool flag_;

    constexpr atomic_flag() noexcept : flag_(false) {}

    atomic_flag& operator=(const atomic_flag&) volatile = delete;

    constexpr bool test_and_set(memory_order order = memory_order_seq_cst) noexcept
    {
        return __atomic_test_and_set(&flag_, static_cast<i32>(order));
    }

    void clear(memory_order order = memory_order_seq_cst) noexcept
    {
        ASSERT_FALSE(
            order == memory_order_consume || order == memory_order_acquire ||
                order == memory_order_acq_rel,
            "Undefined behavior"
        );
        __atomic_clear(&flag_, static_cast<i32>(order));
    }

    bool test(memory_order order = memory_order_seq_cst) const noexcept
    {
        ASSERT_FALSE(
            order == memory_order_release || order == memory_order_acq_rel, "Undefined behavior"
        );
        return __atomic_load_n(&flag_, static_cast<i32>(order));
    }
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
