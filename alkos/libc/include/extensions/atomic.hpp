#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_

#include <assert.h>
#include <stdint.h>
#include <extensions/defines.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/type_traits.hpp>

#define __DEFINE_VOLATILE_PAIR(declaration, body) \
    declaration noexcept body declaration volatile noexcept body

namespace std
{

// ------------------------------
// std::memory_order
// ------------------------------

enum class memory_order : int {
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
// Internal implementation
// ------------------------------

namespace internal
{

template <typename T>
    requires std::is_trivially_copyable_v<T> && std::is_copy_constructible_v<T> &&
             std::is_move_constructible_v<T> && std::is_copy_assignable_v<T> &&
             std::is_move_assignable_v<T> && std::is_same_v<T, std::remove_cv_t<T>>
struct AtomicBase : template_lib::NoCopy {
    using value_type = T;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr AtomicBase() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : value_()
    {
    }

    constexpr AtomicBase(T desired) noexcept : value_(desired) {};

    // ------------------------------
    // operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator=(T desired), {
        store(desired);
        return desired;
    })

    // ------------------------------
    // Store
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        FORCE_INLINE_F void store(T desired, memory_order order = memory_order_seq_cst), {
            ASSERT_FALSE(
                order == memory_order_consume || order == memory_order_acquire ||
                    order == memory_order_acq_rel,
                "Undefined behavior"
            );
            __atomic_store_n(&value_, desired, static_cast<int>(order));
        }
    )

    // ------------------------------
    // Load
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T load(memory_order order = memory_order_seq_cst) const, {
            ASSERT_FALSE(
                order == memory_order_release || order == memory_order_acq_rel, "Undefined behavior"
            );
            return __atomic_load_n(&value_, static_cast<int>(order));
        }
    )

    // ------------------------------
    // Conversion operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(operator T() const, { return load(); })

    // ------------------------------
    // Exchange
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T exchange(T desired, memory_order order = memory_order_seq_cst), {
            ASSERT_FALSE(
                order == memory_order_consume || order == memory_order_acquire ||
                    order == memory_order_acq_rel,
                "Undefined behavior"
            );
            return __atomic_exchange_n(&value_, desired, static_cast<int>(order));
        }
    )

    // ------------------------------
    // Compare and exchange
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
            T& expected, T desired, memory_order success_order = memory_order_seq_cst,
            memory_order failure_order = memory_order_seq_cst
        ),
        {
            ASSERT_FALSE(
                failure_order == memory_order_release || failure_order == memory_order_acq_rel,
                "Undefined behavior"
            );
            return __atomic_compare_exchange_n(
                &value_, &expected, desired, true, static_cast<int>(success_order),
                static_cast<int>(failure_order)
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
            T& expected, T desired, memory_order order = memory_order_seq_cst
        ),
        {
            int success_order = static_cast<int>(order);
            int failure_order;
            if (order == memory_order_acq_rel) {
                failure_order = static_cast<int>(memory_order_acquire);
            } else if (order == memory_order_release) {
                failure_order = static_cast<int>(memory_order_relaxed);
            } else {
                failure_order = static_cast<int>(order);
            }

            return __atomic_compare_exchange_n(
                &value_, &expected, desired, true, success_order, failure_order
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
            T& expected, T desired, memory_order success_order = memory_order_seq_cst,
            memory_order failure_order = memory_order_seq_cst
        ),
        {
            ASSERT_FALSE(
                failure_order == memory_order_release || failure_order == memory_order_acq_rel,
                "Undefined behavior"
            );
            return __atomic_compare_exchange_n(
                &value_, &expected, desired, false, static_cast<int>(success_order),
                static_cast<int>(failure_order)
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
            T& expected, T desired, memory_order order = memory_order_seq_cst
        ),
        {
            int success_order = static_cast<int>(order);
            int failure_order;
            if (order == memory_order_acq_rel) {
                failure_order = static_cast<int>(memory_order_acquire);
            } else if (order == memory_order_release) {
                failure_order = static_cast<int>(memory_order_relaxed);
            } else {
                failure_order = static_cast<int>(order);
            }

            return __atomic_compare_exchange_n(
                &value_, &expected, desired, false, success_order, failure_order
            );
        }
    )

    // ------------------------------
    // Constants
    // ------------------------------

    static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(value_type), 0);

    __DEFINE_VOLATILE_PAIR(NODISCARD FORCE_INLINE_F bool is_lock_free() const, {
        return __atomic_is_lock_free(sizeof(value_type), &value_);
    })

    protected:
    value_type value_;
};

}  // namespace internal

// ------------------------------
// std::atomic
// ------------------------------

template <typename T>
struct atomic : public internal::AtomicBase<T> {
    private:
    using base_type = internal::AtomicBase<T>;

    public:
    using value_type = typename base_type::value_type;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : base_type()
    {
    }

    constexpr atomic(T desired) noexcept : base_type(desired) {};

    // ------------------------------
    // Operators
    // ------------------------------

    using base_type::operator value_type;
    using base_type::operator=;
};

// ------------------------------
// std::atomic<T*>
// ------------------------------

template <typename T>
struct atomic<T*> : public internal::AtomicBase<T*> {
    private:
    using base_type = internal::AtomicBase<T*>;

    public:
    using value_type      = typename base_type::value_type;
    using difference_type = ptrdiff_t;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept = default;
    constexpr atomic(T* desired) noexcept : base_type(desired) {}

    // ------------------------------
    // Pointer arithmetic operations
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T* fetch_add(
            ptrdiff_t arg, memory_order order = memory_order_seq_cst
        ),
        {
            return static_cast<T*>(
                __atomic_fetch_add(&this->value_, arg * sizeof(T), static_cast<int>(order))
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T* fetch_sub(
            ptrdiff_t arg, memory_order order = memory_order_seq_cst
        ),
        {
            return static_cast<T*>(
                __atomic_fetch_sub(&this->value_, arg * sizeof(T), static_cast<int>(order))
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(T* operator++(int), { return fetch_add(1); })

    __DEFINE_VOLATILE_PAIR(T* operator--(int), { return fetch_sub(1); })

    __DEFINE_VOLATILE_PAIR(T* operator++(), { return fetch_add(1) + 1; })

    __DEFINE_VOLATILE_PAIR(T* operator--(), { return fetch_sub(1) - 1; })

    __DEFINE_VOLATILE_PAIR(T* operator+=(ptrdiff_t arg), { return fetch_add(arg) + arg; })

    __DEFINE_VOLATILE_PAIR(T* operator-=(ptrdiff_t arg), { return fetch_sub(arg) - arg; })
};

// ------------------------------
// Aliases
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
// std::atomic_flag
// ------------------------------

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Missing implementations:
 * - wait
 * - notify_one
 * - notify_all
 */

class atomic_flag : public template_lib::NoCopy
{
    public:
    constexpr atomic_flag() noexcept
    {
        __atomic_clear(&flag_, static_cast<int>(memory_order_relaxed));
    }

    atomic_flag& operator=(const atomic_flag&) volatile = delete;

    __DEFINE_VOLATILE_PAIR(FORCE_INLINE_F void clear(memory_order order = memory_order_seq_cst), {
        ASSERT_FALSE(
            order == memory_order_consume || order == memory_order_acquire ||
                order == memory_order_acq_rel,
            "Undefined behavior"
        );
        __atomic_clear(&flag_, static_cast<int>(order));
    })

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test_and_set(memory_order order = memory_order_seq_cst),
        { return __atomic_test_and_set(&flag_, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test(memory_order order = memory_order_seq_cst) const, {
            ASSERT_FALSE(
                order == memory_order_release || order == memory_order_acq_rel, "Undefined behavior"
            );
            return __atomic_load_n(&flag_, static_cast<int>(order));
        }
    )

    private:
    bool flag_;
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
