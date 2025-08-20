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
// Memory synchronization ordering
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

FORCE_INLINE_F void atomic_thread_fence(memory_order m)
{
    __atomic_thread_fence(static_cast<int>(m));
}

FORCE_INLINE_F void atomic_signal_fence(memory_order m)
{
    __atomic_signal_fence(static_cast<int>(m));
}

template <typename T>
inline T kill_dependency(T value) noexcept
{
    // This function is used to eliminate compiler optimizations
    // that might remove dependencies between memory accesses.
    T ret(value);
    return ret;
}

// ------------------------------
// Internal implementation
// ------------------------------

namespace internal
{

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Missing implementations:
 * - wait
 * - notify_one
 * - notify_all
 */

template <typename T, size_t Alignement>
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
            ASSERT_NEQ(order, memory_order_consume);
            ASSERT_NEQ(order, memory_order_acquire);
            ASSERT_NEQ(order, memory_order_acq_rel);
            __atomic_store_n(&value_, desired, static_cast<int>(order));
        }
    )

    // ------------------------------
    // Load
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T load(memory_order order = memory_order_seq_cst) const, {
            ASSERT_NEQ(order, memory_order_release);
            ASSERT_NEQ(order, memory_order_acq_rel);
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
            ASSERT_NEQ(order, memory_order_consume);
            ASSERT_NEQ(order, memory_order_acquire);
            ASSERT_NEQ(order, memory_order_acq_rel);
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
            ASSERT_NEQ(failure_order, memory_order_release);
            ASSERT_NEQ(failure_order, memory_order_acq_rel);
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
        { return compare_exchange_weak(expected, desired, order, GetFailureOrder_(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
            T& expected, T desired, memory_order success_order = memory_order_seq_cst,
            memory_order failure_order = memory_order_seq_cst
        ),
        {
            ASSERT_NEQ(failure_order, memory_order_release);
            ASSERT_NEQ(failure_order, memory_order_acq_rel);
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
        { return compare_exchange_strong(expected, desired, order, GetFailureOrder_(order)); }
    )

    // ------------------------------
    // Constants
    // ------------------------------

    static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(value_type), 0);

    __DEFINE_VOLATILE_PAIR(NODISCARD FORCE_INLINE_F bool is_lock_free() const, {
        return __atomic_is_lock_free(sizeof(value_type), reinterpret_cast<void*>(-Alignement));
    })

    private:
    static constexpr memory_order GetFailureOrder_(memory_order order) noexcept
    {
        return order == memory_order_acq_rel   ? memory_order_acquire
               : order == memory_order_release ? memory_order_relaxed
                                               : order;
    }

    protected:
    alignas(Alignement) value_type value_{};
};

template <typename T>
inline constexpr size_t IntegralAlignment = sizeof(T) > alignof(T) ? sizeof(T) : alignof(T);

template <typename T>
    requires std::is_pointer_v<T>
using PointerBase = AtomicBase<T, alignof(T)>;

template <typename T>
using IntegralBase = AtomicBase<T, IntegralAlignment<T>>;

template <typename T>
using FloatingPointBase = AtomicBase<T, alignof(T)>;

}  // namespace internal

#define ATOMIC_VAR_INIT(value) {value}

// ------------------------------
// std::atomic
// ------------------------------

template <typename T>
struct atomic : internal::AtomicBase<T, alignof(T)> {
    private:
    using base_type = internal::AtomicBase<T, alignof(T)>;

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

    constexpr atomic(T desired) noexcept : base_type(desired) {}

    using base_type::operator=;
};

// ------------------------------
// std::atomic<T*>
// ------------------------------

template <typename T>
struct atomic<T*> : internal::PointerBase<T*> {
    private:
    using base_type = internal::PointerBase<T*>;

    public:
    using value_type      = typename base_type::value_type;
    using difference_type = ptrdiff_t;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : base_type()
    {
    }

    constexpr atomic(T* desired) noexcept : base_type(desired) {}

    using base_type::operator=;

    // ------------------------------
    // Pointer arithmetic operations
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T* fetch_add(
            difference_type arg, memory_order order = memory_order_seq_cst
        ),
        {
            return static_cast<T*>(
                __atomic_fetch_add(&value_, GetByteOffset_(arg), static_cast<int>(order))
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T* fetch_sub(
            difference_type arg, memory_order order = memory_order_seq_cst
        ),
        {
            return static_cast<T*>(
                __atomic_fetch_sub(&value_, GetByteOffset_(arg), static_cast<int>(order))
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(T* operator++(int), { return fetch_add(1); })

    __DEFINE_VOLATILE_PAIR(T* operator--(int), { return fetch_sub(1); })

    __DEFINE_VOLATILE_PAIR(T* operator++(), { return AddFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T* operator--(), { return SubFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T* operator+=(difference_type arg), { return AddFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T* operator-=(difference_type arg), { return SubFetch_(arg); })

    static constexpr bool is_always_lock_free = __GCC_ATOMIC_POINTER_LOCK_FREE == 2;

    private:
    static constexpr difference_type GetByteOffset_(difference_type arg) { return arg * sizeof(T); }

    T* AddFetch_(difference_type arg)
    {
        return static_cast<T*>(
            __atomic_add_fetch(&value_, GetByteOffset_(arg), static_cast<int>(memory_order_seq_cst))
        );
    }

    T* SubFetch_(difference_type arg)
    {
        return static_cast<T*>(
            __atomic_sub_fetch(&value_, GetByteOffset_(arg), static_cast<int>(memory_order_seq_cst))
        );
    }

    using base_type::value_;
};

// ------------------------------
// Specializations for integral types
// ------------------------------

template <integral T>
    requires(!std::is_same_v<bool, std::remove_cv_t<T>>)
struct atomic<T> : internal::IntegralBase<T> {
    private:
    using base_type = internal::IntegralBase<T>;

    public:
    using value_type      = typename base_type::value_type;
    using difference_type = value_type;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : base_type()
    {
    }

    constexpr atomic(T desired) noexcept : base_type(desired) {}

    using base_type::operator=;

    // ------------------------------
    // Arithmetic operations
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_add(T arg, memory_order order = memory_order_seq_cst),
        { return __atomic_fetch_add(&value_, arg, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_sub(T arg, memory_order order = memory_order_seq_cst),
        { return __atomic_fetch_sub(&value_, arg, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_and(T arg, memory_order order = memory_order_seq_cst),
        { return __atomic_fetch_and(&value_, arg, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_or(T arg, memory_order order = memory_order_seq_cst),
        { return __atomic_fetch_or(&value_, arg, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_xor(T arg, memory_order order = memory_order_seq_cst),
        { return __atomic_fetch_xor(&value_, arg, static_cast<int>(order)); }
    )

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator+=(T arg), { return AddFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator-=(T arg), { return SubFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator&=(T arg), { return AndFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator|=(T arg), { return OrFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator^=(T arg), { return XorFetch_(arg); })

    // ------------------------------
    // Increment/decrement operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator++(int), { return fetch_add(1); })

    __DEFINE_VOLATILE_PAIR(T operator--(int), { return fetch_sub(1); })

    __DEFINE_VOLATILE_PAIR(T operator++(), { return AddFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T operator--(), { return SubFetch_(1); })

    private:
    T AddFetch_(T arg)
    {
        return __atomic_add_fetch(&value_, arg, static_cast<int>(memory_order_seq_cst));
    }

    T SubFetch_(T arg)
    {
        return __atomic_sub_fetch(&value_, arg, static_cast<int>(memory_order_seq_cst));
    }

    T AndFetch_(T arg)
    {
        return __atomic_and_fetch(&value_, arg, static_cast<int>(memory_order_seq_cst));
    }

    T OrFetch_(T arg)
    {
        return __atomic_or_fetch(&value_, arg, static_cast<int>(memory_order_seq_cst));
    }

    T XorFetch_(T arg)
    {
        return __atomic_xor_fetch(&value_, arg, static_cast<int>(memory_order_seq_cst));
    }

    using base_type::value_;
};

// ------------------------------
// Specializations for floating-point types
// ------------------------------

template <floating_point T>
struct atomic<T> : internal::FloatingPointBase<T> {
    private:
    using base_type = internal::FloatingPointBase<T>;

    public:
    using value_type      = typename base_type::value_type;
    using difference_type = value_type;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : base_type()
    {
    }

    constexpr atomic(T desired) noexcept : base_type(desired) {}

    using base_type::operator=;

    // ------------------------------
    // Floating-point arithmetic operations
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_add(T arg, memory_order order = memory_order_seq_cst), {
            T expected = this->load(memory_order_relaxed);
            T desired;
            do {
                desired = expected + arg;
            } while (!this->compare_exchange_weak(expected, desired, order, memory_order_relaxed));
            return expected;
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T fetch_sub(T arg, memory_order order = memory_order_seq_cst), {
            T expected = this->load(memory_order_relaxed);
            T desired;
            do {
                desired = expected - arg;
            } while (!this->compare_exchange_weak(expected, desired, order, memory_order_relaxed));
            return expected;
        }
    )

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator+=(T arg), { return fetch_add(arg) + arg; })

    __DEFINE_VOLATILE_PAIR(T operator-=(T arg), { return fetch_sub(arg) - arg; })
};

// ------------------------------
// Type aliases
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

#define ATOMIC_FLAG_INIT {0}

class atomic_flag : public template_lib::NoCopy
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    constexpr atomic_flag() noexcept = default;

    // Conversion to ATOMIC_FLAG_INIT.
    constexpr atomic_flag(bool state) noexcept
        : flag_(state ? __GCC_ATOMIC_TEST_AND_SET_TRUEVAL : 0)
    {
    }

    atomic_flag& operator=(const atomic_flag&) volatile = delete;

    // ------------------------------
    // Methods
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(FORCE_INLINE_F void clear(memory_order order = memory_order_seq_cst), {
        ASSERT_NEQ(order, memory_order_consume);
        ASSERT_NEQ(order, memory_order_acquire);
        ASSERT_NEQ(order, memory_order_acq_rel);
        __atomic_clear(&flag_, static_cast<int>(order));
    })

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test_and_set(memory_order order = memory_order_seq_cst),
        { return __atomic_test_and_set(&flag_, static_cast<int>(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test(memory_order order = memory_order_seq_cst) const, {
            ASSERT_NEQ(order, memory_order_release);
            ASSERT_NEQ(order, memory_order_acq_rel);
            return __atomic_load_n(&flag_, static_cast<int>(order)) ==
                   __GCC_ATOMIC_TEST_AND_SET_TRUEVAL;
        }
    )

    // ------------------------------
    // Private members
    // ------------------------------

    private:
    byte flag_{};
};

// ------------------------------
// Operations on atomic types
// ------------------------------

// atomic_is_lock_free
template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_is_lock_free(const atomic<T>* obj) noexcept
{
    return obj->is_lock_free();
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_is_lock_free(const volatile atomic<T>* obj) noexcept
{
    return obj->is_lock_free();
}

// atomic_store / atomic_store_explicit
template <typename T>
FORCE_INLINE_F void atomic_store(atomic<T>* obj, T desired) noexcept
{
    obj->store(desired);
}

template <typename T>
FORCE_INLINE_F void atomic_store(volatile atomic<T>* obj, T desired) noexcept
{
    obj->store(desired);
}

template <typename T>
FORCE_INLINE_F void atomic_store_explicit(atomic<T>* obj, T desired, memory_order order) noexcept
{
    obj->store(desired, order);
}

template <typename T>
FORCE_INLINE_F void atomic_store_explicit(
    volatile atomic<T>* obj, T desired, memory_order order
) noexcept
{
    obj->store(desired, order);
}

// atomic_load / atomic_load_explicit
template <typename T>
NODISCARD FORCE_INLINE_F T atomic_load(const atomic<T>* obj) noexcept
{
    return obj->load();
}

template <typename T>
NODISCARD FORCE_INLINE_F T atomic_load(const volatile atomic<T>* obj) noexcept
{
    return obj->load();
}

template <typename T>
NODISCARD FORCE_INLINE_F T atomic_load_explicit(const atomic<T>* obj, memory_order order) noexcept
{
    return obj->load(order);
}

template <typename T>
NODISCARD FORCE_INLINE_F T
atomic_load_explicit(const volatile atomic<T>* obj, memory_order order) noexcept
{
    return obj->load(order);
}

// atomic_exchange / atomic_exchange_explicit
template <typename T>
NODISCARD FORCE_INLINE_F T atomic_exchange(atomic<T>* obj, T desired) noexcept
{
    return obj->exchange(desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F T atomic_exchange(volatile atomic<T>* obj, T desired) noexcept
{
    return obj->exchange(desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F T
atomic_exchange_explicit(atomic<T>* obj, T desired, memory_order order) noexcept
{
    return obj->exchange(desired, order);
}

template <typename T>
NODISCARD FORCE_INLINE_F T
atomic_exchange_explicit(volatile atomic<T>* obj, T desired, memory_order order) noexcept
{
    return obj->exchange(desired, order);
}

// atomic_compare_exchange_weak / atomic_compare_exchange_weak_explicit
template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak(
    atomic<T>* obj, T* expected, T desired
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak(
    volatile atomic<T>* obj, T* expected, T desired
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak_explicit(
    atomic<T>* obj, T* expected, T desired, memory_order success, memory_order failure
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired, success, failure);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak_explicit(
    volatile atomic<T>* obj, T* expected, T desired, memory_order success, memory_order failure
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired, success, failure);
}

// atomic_compare_exchange_strong / atomic_compare_exchange_strong_explicit
template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong(
    atomic<T>* obj, T* expected, T desired
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong(
    volatile atomic<T>* obj, T* expected, T desired
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong_explicit(
    atomic<T>* obj, T* expected, T desired, memory_order success, memory_order failure
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired, success, failure);
}

template <typename T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong_explicit(
    volatile atomic<T>* obj, T* expected, T desired, memory_order success, memory_order failure
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired, success, failure);
}

// atomic_fetch_add / atomic_fetch_add_explicit
template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_add(
    atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg
) noexcept
{
    return obj->fetch_add(arg);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_add(
    volatile atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg
) noexcept
{
    return obj->fetch_add(arg);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_add_explicit(
    atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg, memory_order order
) noexcept
{
    return obj->fetch_add(arg, order);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_add_explicit(
    volatile atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg,
    memory_order order
) noexcept
{
    return obj->fetch_add(arg, order);
}

// atomic_fetch_sub / atomic_fetch_sub_explicit
template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_sub(
    atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg
) noexcept
{
    return obj->fetch_sub(arg);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_sub(
    volatile atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg
) noexcept
{
    return obj->fetch_sub(arg);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_sub_explicit(
    atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg, memory_order order
) noexcept
{
    return obj->fetch_sub(arg, order);
}

template <typename T>
    requires(integral<T> || std::is_pointer_v<T>)
NODISCARD FORCE_INLINE_F T atomic_fetch_sub_explicit(
    volatile atomic<T>* obj, std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, T> arg,
    memory_order order
) noexcept
{
    return obj->fetch_sub(arg, order);
}

// atomic_fetch_and / atomic_fetch_and_explicit
template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_and(atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_and(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_and(volatile atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_and(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_and_explicit(atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_and(arg, order);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_and_explicit(volatile atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_and(arg, order);
}

// atomic_fetch_or / atomic_fetch_or_explicit
template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_or(atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_or(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_or(volatile atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_or(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_or_explicit(atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_or(arg, order);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_or_explicit(volatile atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_or(arg, order);
}

// atomic_fetch_xor / atomic_fetch_xor_explicit
template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_xor(atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_xor(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T atomic_fetch_xor(volatile atomic<T>* obj, T arg) noexcept
{
    return obj->fetch_xor(arg);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_xor_explicit(atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_xor(arg, order);
}

template <integral T>
NODISCARD FORCE_INLINE_F T
atomic_fetch_xor_explicit(volatile atomic<T>* obj, T arg, memory_order order) noexcept
{
    return obj->fetch_xor(arg, order);
}

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
