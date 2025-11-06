#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_

#include <assert.h>
#include <stdint.h>
#include <extensions/bit.hpp>
#include <extensions/defines.hpp>
#include <extensions/memory.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/type_traits_ext.hpp>
#include <todo.hpp>

TODO_WHEN_CPP_REFLECTION
// TODO: Refactor to use compile-time reflection when available
#define __DEFINE_VOLATILE_PAIR(declaration, ...) \
    declaration noexcept __VA_ARGS__ declaration volatile noexcept __VA_ARGS__

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

template <typename T>
inline constexpr size_t IntegralAlignment = sizeof(T) > alignof(T) ? sizeof(T) : alignof(T);

template <typename T>
concept AtomicIntegral = integral<T> && !std::is_same_v<bool, std::remove_cv_t<T>>;

template <typename T>
inline constexpr bool IsAtomicObject = !integral<T> && !floating_point<T> && !std::is_pointer_v<T>;

template <typename T>
concept IsFetchAddSupported = requires(T *obj, T val) { __atomic_fetch_add(obj, val, 0); };

template <typename T>
concept IsFetchSubSupported = requires(T *obj, T val) { __atomic_fetch_sub(obj, val, 0); };

template <typename T>
inline constexpr bool HasPadding = !has_unique_object_representations_v<T> &&
                                   !(floating_point<T> && !is_same_v<remove_cv_t<T>, long double>);

template <typename T>
FORCE_INLINE_F constexpr T *ClearPadding(T &obj) noexcept
{
    auto *ptr = std::addressof(obj);

    if constexpr (HasPadding<T>) {
        __builtin_clear_padding(ptr);
    }

    return ptr;
}

// ------------------------------
// internal::AtomicBuiltin
// ------------------------------

template <typename T, bool kAtomicRef = false>
struct AtomicImpl {
    private:
    using value_type      = std::conditional_t<kAtomicRef, std::remove_volatile_t<T>, T>;
    using difference_type = std::conditional_t<std::is_pointer_v<T>, ptrdiff_t, value_type>;

    public:
    // Store operations
    template <typename U>
    FORCE_INLINE_F static void store(U *obj, value_type desired, memory_order order) noexcept
    {
        ASSERT_NEQ(order, memory_order_consume);
        ASSERT_NEQ(order, memory_order_acquire);
        ASSERT_NEQ(order, memory_order_acq_rel);

        if constexpr (IsAtomicObject<T> || floating_point<T>) {
            __atomic_store(obj, ClearPadding(desired), static_cast<int>(order));
        } else {
            __atomic_store_n(obj, desired, static_cast<int>(order));
        }
    }

    // Load operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type load(const U *obj, memory_order order) noexcept
    {
        ASSERT_NEQ(order, memory_order_release);
        ASSERT_NEQ(order, memory_order_acq_rel);

        if constexpr (IsAtomicObject<T> || floating_point<T>) {
            alignas(T) byte buffer[sizeof(T)];
            auto *ptr = reinterpret_cast<value_type *>(buffer);
            __atomic_load(obj, ptr, static_cast<int>(order));
            return *ptr;
        } else {
            return __atomic_load_n(obj, static_cast<int>(order));
        }
    }

    // Exchange operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type exchange(
        U *obj, value_type desired, memory_order order
    ) noexcept
    {
        if constexpr (IsAtomicObject<T> || floating_point<T>) {
            alignas(T) byte buffer[sizeof(T)];
            auto *ptr = reinterpret_cast<value_type *>(buffer);
            __atomic_exchange(obj, ClearPadding(desired), ptr, static_cast<int>(order));
            return *ptr;
        } else {
            return __atomic_exchange_n(obj, desired, static_cast<int>(order));
        }
    }

    // Compare exchange operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static bool compare_exchange_weak(
        U *obj, value_type &expected, value_type &desired, memory_order success_order,
        memory_order failure_order
    ) noexcept
    {
        return compare_exchange_(*obj, expected, desired, true, success_order, failure_order);
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static bool compare_exchange_strong(
        U *obj, value_type &expected, value_type &desired, memory_order success_order,
        memory_order failure_order
    ) noexcept
    {
        return compare_exchange_(*obj, expected, desired, false, success_order, failure_order);
    }

    // Arithmetic operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type fetch_add(
        U *obj, difference_type arg, memory_order order
    ) noexcept
    {
        if constexpr (IsFetchAddSupported<T>) {
            return __atomic_fetch_add(obj, arg, static_cast<int>(order));
        } else {
            value_type expected = load(obj, memory_order_relaxed);
            value_type desired;
            do {
                desired = expected + arg;
            } while (!compare_exchange_weak(obj, expected, desired, order, memory_order_relaxed));
            return expected;
        }
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type fetch_sub(
        U *obj, difference_type arg, memory_order order
    ) noexcept
    {
        if constexpr (IsFetchSubSupported<T>) {
            return __atomic_fetch_sub(obj, arg, static_cast<int>(order));
        } else {
            value_type expected = load(obj, memory_order_relaxed);
            value_type desired;
            do {
                desired = expected - arg;
            } while (!compare_exchange_weak(obj, expected, desired, order, memory_order_relaxed));
            return expected;
        }
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type add_fetch(
        U *obj, difference_type arg, memory_order order
    ) noexcept
    {
        return __atomic_add_fetch(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type sub_fetch(
        U *obj, difference_type arg, memory_order order
    ) noexcept
    {
        return __atomic_sub_fetch(obj, arg, static_cast<int>(order));
    }

    // Bitwise operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type fetch_and(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_fetch_and(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type fetch_or(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_fetch_or(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type fetch_xor(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_fetch_xor(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type and_fetch(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_and_fetch(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type or_fetch(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_or_fetch(obj, arg, static_cast<int>(order));
    }

    template <typename U>
    NODISCARD FORCE_INLINE_F static value_type xor_fetch(
        U *obj, value_type arg, memory_order order
    ) noexcept
    {
        return __atomic_xor_fetch(obj, arg, static_cast<int>(order));
    }

    // Lock-free operations
    template <size_t kSize, size_t kAlignment>
    NODISCARD FORCE_INLINE_F static bool is_lock_free() noexcept
    {
        return __atomic_is_lock_free(kSize, reinterpret_cast<void *>(-kAlignment));
    }

    // Flag operations
    template <typename U>
    NODISCARD FORCE_INLINE_F static bool test_and_set(U *obj, memory_order order) noexcept
    {
        return __atomic_test_and_set(obj, static_cast<int>(order));
    }

    template <typename U>
    FORCE_INLINE_F static void clear(U *obj, memory_order order) noexcept
    {
        __atomic_clear(obj, static_cast<int>(order));
    }

    private:
    template <typename U>
    NODISCARD FORCE_INLINE_F static bool compare_exchange_(
        U &obj, value_type &expected, value_type &desired, bool weak, memory_order success_order,
        memory_order failure_order
    ) noexcept
    {
        ASSERT_NEQ(failure_order, memory_order_release);
        ASSERT_NEQ(failure_order, memory_order_acq_rel);

        if constexpr (!IsAtomicObject<T> && !floating_point<T>) {
            return __atomic_compare_exchange_n(
                &obj, &expected, desired, weak, static_cast<int>(success_order),
                static_cast<int>(failure_order)
            );
        } else {
            T *ptr = std::addressof(obj);

            if constexpr (!HasPadding<value_type>) {
                return __atomic_compare_exchange(
                    ptr, std::addressof(expected), std::addressof(desired), weak,
                    static_cast<int>(success_order), static_cast<int>(failure_order)
                );
            } else {
                // Only allowed to copy on failure
                value_type expected_copy = expected;
                // Clear padding so that comparison is done on value representation
                value_type *expected_ptr = ClearPadding(expected_copy);

                // Clear padding in case of success
                value_type *desired_ptr = ClearPadding(desired);

                if constexpr (!kAtomicRef) {
                    if (__atomic_compare_exchange(
                            ptr, expected_ptr, desired_ptr, weak, static_cast<int>(success_order),
                            static_cast<int>(failure_order)
                        )) {
                        return true;
                    }

                    memcpy(std::addressof(expected), expected_ptr, sizeof(value_type));
                    return false;
                } else {
                    size_t i = 0;
                    while (true) {
                        value_type original = expected_copy;

                        if (__atomic_compare_exchange(
                                ptr, expected_ptr, desired_ptr, weak,
                                static_cast<int>(success_order), static_cast<int>(failure_order)
                            )) {
                            return true;
                        }

                        value_type current = expected_copy;

                        // Compare value representations
                        if (memcmp(
                                ClearPadding(original), ClearPadding(current), sizeof(value_type)
                            )) {
                            // True failure - the value changed
                            memcpy(std::addressof(expected), expected_ptr, sizeof(value_type));
                            return false;
                        }

                        // First CAE fails because the actual value has non-zero padding.
                        // Second CAE fails because another thread stored the same value,
                        // but now with padding cleared. Third CAE succeeds.
                        // We will never need to loop a fourth time, because any value
                        // written by another thread (whether via store, exchange or
                        // compare_exchange) will have had its padding cleared.
                        ++i;
                        ASSERT_LT(i, 3, "Loop shouldn't iterate more than 3 times");
                    }
                }
            }
        }
    }
};

// ------------------------------
// internal::AtomicBase<T, Alignment>
// ------------------------------

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Missing implementations:
 * - wait
 * - notify_one
 * - notify_all
 */

template <typename T, size_t Alignment>
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

    constexpr AtomicBase(T desired) noexcept { value_ = *ClearPadding(desired); };

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
        FORCE_INLINE_F void store(T desired, memory_order order = memory_order_seq_cst),
        { internal::AtomicImpl<T>::store(&value_, desired, order); }
    )

    // ------------------------------
    // Load
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T load(memory_order order = memory_order_seq_cst) const,
        { return internal::AtomicImpl<T>::load(&value_, order); }
    )

    // ------------------------------
    // Conversion operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(operator T() const, { return load(); })

    // ------------------------------
    // Exchange
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T exchange(T desired, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::exchange(&value_, desired, order); }
    )

    // ------------------------------
    // Compare and exchange
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
            T &expected, T desired, memory_order success_order, memory_order failure_order
        ),
        {
            return AtomicImpl<T>::compare_exchange_weak(
                &value_, expected, desired, success_order, failure_order
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
            T &expected, T desired, memory_order order = memory_order_seq_cst
        ),
        { return compare_exchange_weak(expected, desired, order, GetFailureOrder_(order)); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
            T &expected, T desired, memory_order success_order, memory_order failure_order
        ),
        {
            return AtomicImpl<T>::compare_exchange_strong(
                &value_, expected, desired, success_order, failure_order
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
            T &expected, T desired, memory_order order = memory_order_seq_cst
        ),
        { return compare_exchange_strong(expected, desired, order, GetFailureOrder_(order)); }
    )

    // ------------------------------
    // Constants
    // ------------------------------

    static constexpr bool is_always_lock_free =
        __atomic_always_lock_free(sizeof(value_type), nullptr);

    TODO_LIBATOMIC
    static_assert(is_always_lock_free, "Only lock-free atomics are supported");

    __DEFINE_VOLATILE_PAIR(NODISCARD FORCE_INLINE_F bool is_lock_free() const, {
        return internal::AtomicImpl<T>::template is_lock_free<sizeof(T), Alignment>();
    })

    private:
    static constexpr memory_order GetFailureOrder_(memory_order order) noexcept
    {
        return order == memory_order_acq_rel   ? memory_order_acquire
               : order == memory_order_release ? memory_order_relaxed
                                               : order;
    }

    protected:
    alignas(Alignment) value_type value_{};
};

template <typename T>
    requires std::is_pointer_v<T>
using PointerBase = AtomicBase<T, alignof(T)>;

template <typename T>
using IntegralBase = AtomicBase<T, IntegralAlignment<T>>;

template <typename T>
using FloatingPointBase = AtomicBase<T, alignof(T)>;

// ------------------------------
// internal::AtomicRefBase<T>
// ------------------------------

TODO_LIBCPP_COMPLIANCE
/**
 * TODO: Missing implementations:
 * - wait
 * - notify_one
 * - notify_all
 */

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_trivially_copyable_v<T>
struct AtomicRefBase {
    private:
    // 1/2/4/8/16 byte types should have alignment at least their size
    static constexpr size_t MinAlignment_ = !IsPowerOfTwo(sizeof(T)) || sizeof(T) > 16 ? 0
                                                                                       : sizeof(T);

    public:
    using value_type = std::remove_cv_t<T>;

    static constexpr bool is_always_lock_free =
        __atomic_always_lock_free(sizeof(value_type), nullptr);

    static_assert(is_always_lock_free || !std::is_volatile_v<T>, "The program is ill-formed");

    TODO_LIBATOMIC
    static_assert(is_always_lock_free, "Only lock-free atomics are supported");

    static constexpr size_t required_alignment = MinAlignment_ > alignof(value_type)
                                                     ? MinAlignment_
                                                     : alignof(value_type);

    // ------------------------------
    // Struct creation
    // ------------------------------

    AtomicRefBase()                                 = delete;
    AtomicRefBase &operator=(const AtomicRefBase &) = delete;

    explicit AtomicRefBase(T &obj) : ptr_(std::addressof(obj))
    {
        ASSERT_TRUE(
            IsAligned(ptr_, required_alignment),
            "The pointer must be aligned to its required alignment"
        );
    }

    AtomicRefBase(const AtomicRefBase &ref) noexcept = default;

    // ------------------------------
    // Operators
    // ------------------------------

    value_type operator=(value_type desired) const noexcept
        requires(!std::is_const_v<T>)
    {
        store(desired);
        return desired;
    }

    // ------------------------------
    // Conversion
    // ------------------------------

    operator value_type() const noexcept { return load(); }

    // ------------------------------
    // Atomic operations
    // ------------------------------

    NODISCARD FORCE_INLINE_F bool is_lock_free() const noexcept
    {
        return AtomicImpl<T, true>::template is_lock_free<sizeof(T), required_alignment>();
    }

    FORCE_INLINE_F void store(
        value_type desired, memory_order order = memory_order_seq_cst
    ) const noexcept
        requires(!std::is_const_v<T>)
    {
        AtomicImpl<T, true>::store(ptr_, desired, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    load(memory_order order = memory_order_seq_cst) const noexcept
    {
        return AtomicImpl<T, true>::load(ptr_, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    exchange(value_type desired, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return AtomicImpl<T, true>::exchange(ptr_, desired, order);
    }

    NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
        value_type &expected, value_type desired, memory_order success_order,
        memory_order failure_order
    ) const noexcept
        requires(!std::is_const_v<T>)
    {
        return AtomicImpl<T, true>::compare_exchange_weak(
            ptr_, expected, desired, success_order, failure_order
        );
    }

    NODISCARD FORCE_INLINE_F bool compare_exchange_weak(
        value_type &expected, value_type desired, memory_order order = memory_order_seq_cst
    ) const noexcept
        requires(!std::is_const_v<T>)
    {
        return compare_exchange_weak(expected, desired, order, GetFailureOrder_(order));
    }

    NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
        value_type &expected, value_type desired, memory_order success_order,
        memory_order failure_order
    ) const noexcept
        requires(!std::is_const_v<T>)
    {
        return AtomicImpl<T, true>::compare_exchange_strong(
            ptr_, expected, desired, success_order, failure_order
        );
    }

    NODISCARD FORCE_INLINE_F bool compare_exchange_strong(
        value_type &expected, value_type desired, memory_order order = memory_order_seq_cst
    ) const noexcept
        requires(!std::is_const_v<T>)
    {
        return compare_exchange_strong(expected, desired, order, GetFailureOrder_(order));
    }

    NODISCARD FORCE_INLINE_F T *address() const noexcept { return ptr_; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    static constexpr memory_order GetFailureOrder_(memory_order order) noexcept
    {
        return order == memory_order_acq_rel   ? memory_order_acquire
               : order == memory_order_release ? memory_order_relaxed
                                               : order;
    }

    // ------------------------------
    // Private members
    // ------------------------------

    protected:
    T *ptr_;
};

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
    using value_type = base_type::value_type;

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
struct atomic<T *> : internal::PointerBase<T *> {
    private:
    using base_type = internal::PointerBase<T *>;

    public:
    using value_type      = base_type::value_type;
    using difference_type = ptrdiff_t;

    // ------------------------------
    // Struct creation
    // ------------------------------

    constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : base_type()
    {
    }

    constexpr atomic(T *desired) noexcept : base_type(desired) {}

    using base_type::operator=;

    // ------------------------------
    // Pointer arithmetic operations
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            *fetch_add(difference_type arg, memory_order order = memory_order_seq_cst),
        {
            return static_cast<T *>(
                internal::AtomicImpl<T *>::fetch_add(&value_, GetByteOffset_(arg), order)
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            *fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst),
        {
            return static_cast<T *>(
                internal::AtomicImpl<T *>::fetch_sub(&value_, GetByteOffset_(arg), order)
            );
        }
    )

    __DEFINE_VOLATILE_PAIR(T *operator++(int), { return fetch_add(1); })

    __DEFINE_VOLATILE_PAIR(T *operator--(int), { return fetch_sub(1); })

    __DEFINE_VOLATILE_PAIR(T *operator++(), { return AddFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T *operator--(), { return SubFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T *operator+=(difference_type arg), { return AddFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T *operator-=(difference_type arg), { return SubFetch_(arg); })

    static constexpr bool is_always_lock_free = __GCC_ATOMIC_POINTER_LOCK_FREE == 2;

    private:
    static constexpr difference_type GetByteOffset_(difference_type arg) { return arg * sizeof(T); }

    T *AddFetch_(difference_type arg)
    {
        return static_cast<T *>(
            internal::AtomicImpl<T *>::add_fetch(&value_, GetByteOffset_(arg), memory_order_seq_cst)
        );
    }

    T *SubFetch_(difference_type arg)
    {
        return static_cast<T *>(
            internal::AtomicImpl<T *>::sub_fetch(&value_, GetByteOffset_(arg), memory_order_seq_cst)
        );
    }

    using base_type::value_;
};

// ------------------------------
// Specializations for integral types
// ------------------------------

template <internal::AtomicIntegral T>
struct atomic<T> : internal::IntegralBase<T> {
    private:
    using base_type = internal::IntegralBase<T>;

    public:
    using value_type      = base_type::value_type;
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
        NODISCARD FORCE_INLINE_F T
            fetch_add(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_add(&value_, arg, order); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_sub(&value_, arg, order); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            fetch_and(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_and(&value_, arg, order); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            fetch_or(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_or(&value_, arg, order); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            fetch_xor(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_xor(&value_, arg, order); }
    )

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator+=(difference_type arg), { return AddFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator-=(difference_type arg), { return SubFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator&=(difference_type arg), { return AndFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator|=(difference_type arg), { return OrFetch_(arg); })

    __DEFINE_VOLATILE_PAIR(T operator^=(difference_type arg), { return XorFetch_(arg); })

    // ------------------------------
    // Increment/decrement operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator++(int), { return fetch_add(1); })

    __DEFINE_VOLATILE_PAIR(T operator--(int), { return fetch_sub(1); })

    __DEFINE_VOLATILE_PAIR(T operator++(), { return AddFetch_(1); })

    __DEFINE_VOLATILE_PAIR(T operator--(), { return SubFetch_(1); })

    private:
    NODISCARD FORCE_INLINE_F T AddFetch_(difference_type arg)
    {
        return internal::AtomicImpl<T>::add_fetch(&value_, arg, memory_order_seq_cst);
    }

    NODISCARD FORCE_INLINE_F T SubFetch_(difference_type arg)
    {
        return internal::AtomicImpl<T>::sub_fetch(&value_, arg, memory_order_seq_cst);
    }

    NODISCARD FORCE_INLINE_F T AndFetch_(difference_type arg)
    {
        return internal::AtomicImpl<T>::and_fetch(&value_, arg, memory_order_seq_cst);
    }

    NODISCARD FORCE_INLINE_F T OrFetch_(difference_type arg)
    {
        return internal::AtomicImpl<T>::or_fetch(&value_, arg, memory_order_seq_cst);
    }

    NODISCARD FORCE_INLINE_F T XorFetch_(difference_type arg)
    {
        return internal::AtomicImpl<T>::xor_fetch(&value_, arg, memory_order_seq_cst);
    }

    private:
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
    using value_type      = base_type::value_type;
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
        NODISCARD FORCE_INLINE_F T
            fetch_add(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_add(&value_, arg, order); }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F T
            fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst),
        { return internal::AtomicImpl<T>::fetch_sub(&value_, arg, order); }
    )

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(T operator+=(T arg), { return fetch_add(arg) + arg; })

    __DEFINE_VOLATILE_PAIR(T operator-=(T arg), { return fetch_sub(arg) - arg; })

    private:
    using base_type::value_;
};

// ------------------------------
// Type aliases
// ------------------------------

using atomic_bool     = atomic<bool>;
using atomic_char     = atomic<char>;
using atomic_schar    = atomic<signed char>;
using atomic_uchar    = atomic<unsigned char>;
using atomic_short    = atomic<short>;
using atomic_ushort   = atomic<unsigned short>;
using atomic_int      = atomic<int>;
using atomic_uint     = atomic<unsigned int>;
using atomic_long     = atomic<long>;
using atomic_ulong    = atomic<unsigned long>;
using atomic_llong    = atomic<long long>;
using atomic_ullong   = atomic<unsigned long long>;
using atomic_char8_t  = atomic<char8_t>;
using atomic_char16_t = atomic<char16_t>;
using atomic_char32_t = atomic<char32_t>;
using atomic_wchar_t  = atomic<wchar_t>;

using atomic_int8_t   = atomic<int8_t>;
using atomic_uint8_t  = atomic<uint8_t>;
using atomic_int16_t  = atomic<int16_t>;
using atomic_uint16_t = atomic<uint16_t>;
using atomic_int32_t  = atomic<int32_t>;
using atomic_uint32_t = atomic<uint32_t>;
using atomic_int64_t  = atomic<int64_t>;
using atomic_uint64_t = atomic<uint64_t>;

using atomic_int_least8_t   = atomic<int_least8_t>;
using atomic_uint_least8_t  = atomic<uint_least8_t>;
using atomic_int_least16_t  = atomic<int_least16_t>;
using atomic_uint_least16_t = atomic<uint_least16_t>;
using atomic_int_least32_t  = atomic<int_least32_t>;
using atomic_uint_least32_t = atomic<uint_least32_t>;
using atomic_int_least64_t  = atomic<int_least64_t>;
using atomic_uint_least64_t = atomic<uint_least64_t>;

using atomic_int_fast8_t   = atomic<int_fast8_t>;
using atomic_uint_fast8_t  = atomic<uint_fast8_t>;
using atomic_int_fast16_t  = atomic<int_fast16_t>;
using atomic_uint_fast16_t = atomic<uint_fast16_t>;
using atomic_int_fast32_t  = atomic<int_fast32_t>;
using atomic_uint_fast32_t = atomic<uint_fast32_t>;
using atomic_int_fast64_t  = atomic<int_fast64_t>;
using atomic_uint_fast64_t = atomic<uint_fast64_t>;

using atomic_intptr_t  = atomic<intptr_t>;
using atomic_uintptr_t = atomic<uintptr_t>;
using atomic_size_t    = atomic<size_t>;
using atomic_ptrdiff_t = atomic<ptrdiff_t>;
using atomic_intmax_t  = atomic<intmax_t>;
using atomic_uintmax_t = atomic<uintmax_t>;

// ------------------------------
// Non-member functions
// ------------------------------

namespace internal
{
template <typename T, bool kConst = false>
concept IsAtomic = std::is_same_v<std::remove_cv_t<T>, std::atomic<typename T::value_type>> &&
                   std::is_convertible_v<T *, type_traits_ext::conditional_const_t<kConst, T> *>;
}

template <internal::IsAtomic<true> T>
NODISCARD FORCE_INLINE_F bool atomic_is_lock_free(T *obj) noexcept
{
    return obj->is_lock_free();
}

template <internal::IsAtomic T>
FORCE_INLINE_F void atomic_store(T *obj, typename T::value_type desired) noexcept
{
    obj->store(desired);
}

template <internal::IsAtomic T>
FORCE_INLINE_F void atomic_store_explicit(
    T *obj, typename T::value_type desired, memory_order order
) noexcept
{
    obj->store(desired, order);
}

template <internal::IsAtomic<true> T>
NODISCARD FORCE_INLINE_F T atomic_load(T *obj) noexcept
{
    return obj->load();
}

template <internal::IsAtomic<true> T>
NODISCARD FORCE_INLINE_F T atomic_load_explicit(T *obj, memory_order order) noexcept
{
    return obj->load(order);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F T atomic_exchange(T *obj, typename T::value_type desired) noexcept
{
    return obj->exchange(desired);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F T
atomic_exchange_explicit(T *obj, typename T::value_type desired, memory_order order) noexcept
{
    return obj->exchange(desired, order);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak(
    T *obj, typename T::value_type *expected, typename T::value_type desired
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_weak_explicit(
    T *obj, typename T::value_type *expected, typename T::value_type desired, memory_order success,
    memory_order failure
) noexcept
{
    return obj->compare_exchange_weak(*expected, desired, success, failure);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong(
    T *obj, typename T::value_type *expected, typename T::value_type desired
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired);
}

template <internal::IsAtomic T>
NODISCARD FORCE_INLINE_F bool atomic_compare_exchange_strong_explicit(
    T *obj, typename T::value_type *expected, typename T::value_type desired, memory_order success,
    memory_order failure
) noexcept
{
    return obj->compare_exchange_strong(*expected, desired, success, failure);
}

template <internal::IsAtomic T>
    requires(
        internal::AtomicIntegral<typename T::value_type> ||
        floating_point<typename T::value_type> || std::is_pointer_v<typename T::value_type>
    )
NODISCARD FORCE_INLINE_F T atomic_fetch_add(T *obj, typename T::difference_type arg) noexcept
{
    return obj->fetch_add(arg);
}

template <internal::IsAtomic T>
    requires(
        internal::AtomicIntegral<typename T::value_type> ||
        floating_point<typename T::value_type> || std::is_pointer_v<typename T::value_type>
    )
NODISCARD FORCE_INLINE_F T
atomic_fetch_add_explicit(T *obj, typename T::difference_type arg, memory_order order) noexcept
{
    return obj->fetch_add(arg, order);
}

template <internal::IsAtomic T>
    requires(
        internal::AtomicIntegral<typename T::value_type> ||
        floating_point<typename T::value_type> || std::is_pointer_v<typename T::value_type>
    )
NODISCARD FORCE_INLINE_F T atomic_fetch_sub(T *obj, typename T::difference_type arg) noexcept
{
    return obj->fetch_sub(arg);
}

template <internal::IsAtomic T>
    requires(
        internal::AtomicIntegral<typename T::value_type> ||
        floating_point<typename T::value_type> || std::is_pointer_v<typename T::value_type>
    )
NODISCARD FORCE_INLINE_F T
atomic_fetch_sub_explicit(T *obj, typename T::difference_type arg, memory_order order) noexcept
{
    return obj->fetch_sub(arg, order);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T atomic_fetch_and(T *obj, typename T::value_type arg) noexcept
{
    return obj->fetch_and(arg);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T
atomic_fetch_and_explicit(T *obj, typename T::value_type arg, memory_order order) noexcept
{
    return obj->fetch_and(arg, order);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T atomic_fetch_or(T *obj, typename T::value_type arg) noexcept
{
    return obj->fetch_or(arg);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T
atomic_fetch_or_explicit(T *obj, typename T::value_type arg, memory_order order) noexcept
{
    return obj->fetch_or(arg, order);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T atomic_fetch_xor(T *obj, typename T::value_type arg) noexcept
{
    return obj->fetch_xor(arg);
}

template <internal::IsAtomic T>
    requires internal::AtomicIntegral<typename T::value_type>
NODISCARD FORCE_INLINE_F T
atomic_fetch_xor_explicit(T *obj, typename T::value_type arg, memory_order order) noexcept
{
    return obj->fetch_xor(arg, order);
}

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

    atomic_flag &operator=(const atomic_flag &) volatile = delete;

    // ------------------------------
    // Methods
    // ------------------------------

    __DEFINE_VOLATILE_PAIR(FORCE_INLINE_F void clear(memory_order order = memory_order_seq_cst), {
        ASSERT_NEQ(order, memory_order_consume);
        ASSERT_NEQ(order, memory_order_acquire);
        ASSERT_NEQ(order, memory_order_acq_rel);

        using flag_type = std::remove_pointer_t<decltype(&flag_)>;
        internal::AtomicImpl<flag_type>::clear(&flag_, order);
    })

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test_and_set(memory_order order = memory_order_seq_cst), {
            using flag_type = std::remove_pointer_t<decltype(&flag_)>;
            return internal::AtomicImpl<flag_type>::test_and_set(&flag_, order);
        }
    )

    __DEFINE_VOLATILE_PAIR(
        NODISCARD FORCE_INLINE_F bool test(memory_order order = memory_order_seq_cst) const, {
            ASSERT_NEQ(order, memory_order_release);
            ASSERT_NEQ(order, memory_order_acq_rel);

            using flag_type = std::remove_pointer_t<decltype(&flag_)>;
            return internal::AtomicImpl<flag_type>::load(&flag_, order) ==
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
// std::atomic_ref
// ------------------------------

template <typename T>
struct atomic_ref : internal::AtomicRefBase<T> {
    private:
    using base_type = internal::AtomicRefBase<T>;

    public:
    using value_type = base_type::value_type;

    // ------------------------------
    // Struct creation
    // ------------------------------

    explicit atomic_ref(T &ref) noexcept : base_type(ref) {}
    atomic_ref(const atomic_ref &ref) noexcept : base_type(ref) {}

    using base_type::operator=;
};

// ------------------------------
// std::atomic_ref<integral>
// ------------------------------

template <internal::AtomicIntegral T>
struct atomic_ref<T> : internal::AtomicRefBase<T> {
    private:
    using base_type = internal::AtomicRefBase<T>;

    public:
    using value_type      = base_type::value_type;
    using difference_type = value_type;

    static constexpr size_t required_alignment = internal::IntegralAlignment<T>;

    // ------------------------------
    // Struct creation
    // ------------------------------

    explicit atomic_ref(T &ref) noexcept : base_type(ref) {}
    atomic_ref(const atomic_ref &ref) noexcept : base_type(ref) {}

    using base_type::operator=;

    // ------------------------------
    // Arithmetic operations
    // ------------------------------

    NODISCARD FORCE_INLINE_F value_type
    fetch_add(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_add(ptr_, arg, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_sub(ptr_, arg, order);
    }

    // ------------------------------
    // Bitwise operations
    // ------------------------------

    NODISCARD FORCE_INLINE_F value_type
    fetch_and(value_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_and(ptr_, arg, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    fetch_or(value_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_or(ptr_, arg, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    fetch_xor(value_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_xor(ptr_, arg, order);
    }

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    value_type operator+=(difference_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::add_fetch(ptr_, arg, memory_order_seq_cst);
    }

    value_type operator-=(difference_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::sub_fetch(ptr_, arg, memory_order_seq_cst);
    }

    value_type operator&=(value_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::and_fetch(ptr_, arg, memory_order_seq_cst);
    }

    value_type operator|=(value_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::or_fetch(ptr_, arg, memory_order_seq_cst);
    }

    value_type operator^=(value_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::xor_fetch(ptr_, arg, memory_order_seq_cst);
    }

    // ------------------------------
    // Increment/decrement operators
    // ------------------------------

    value_type operator++(int) const noexcept
        requires(!std::is_const_v<T>)
    {
        return fetch_add(1);
    }

    value_type operator--(int) const noexcept
        requires(!std::is_const_v<T>)
    {
        return fetch_sub(1);
    }

    value_type operator++() const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::add_fetch(ptr_, 1, memory_order_seq_cst);
    }

    value_type operator--() const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::sub_fetch(ptr_, 1, memory_order_seq_cst);
    }

    private:
    using base_type::ptr_;
};

// ------------------------------
// std::atomic_ref<floating_point>
// ------------------------------

template <floating_point T>
struct atomic_ref<T> : internal::AtomicRefBase<T> {
    private:
    using base_type = internal::AtomicRefBase<T>;

    public:
    using value_type      = base_type::value_type;
    using difference_type = value_type;

    static constexpr size_t required_alignment = alignof(T);

    // ------------------------------
    // Struct creation
    // ------------------------------

    explicit atomic_ref(T &ref) noexcept : base_type(ref) {}
    atomic_ref(const atomic_ref &ref) noexcept : base_type(ref) {}

    using base_type::operator=;

    // ------------------------------
    // Floating-point arithmetic operations
    // ------------------------------

    NODISCARD FORCE_INLINE_F value_type
    fetch_add(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_add(ptr_, arg, order);
    }

    NODISCARD FORCE_INLINE_F value_type
    fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T>)
    {
        return internal::AtomicImpl<T, true>::fetch_sub(ptr_, arg, order);
    }

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    value_type operator+=(difference_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return fetch_add(arg) + arg;
    }

    value_type operator-=(difference_type arg) const noexcept
        requires(!std::is_const_v<T>)
    {
        return fetch_sub(arg) - arg;
    }

    private:
    using base_type::ptr_;
};

// ------------------------------
// std::atomic_ref<T*>
// ------------------------------

template <typename T>
struct atomic_ref<T *> : internal::AtomicRefBase<T *> {
    private:
    using base_type = internal::AtomicRefBase<T *>;

    public:
    using value_type      = base_type::value_type;
    using difference_type = ptrdiff_t;

    static constexpr bool is_always_lock_free  = __GCC_ATOMIC_POINTER_LOCK_FREE == 2;
    static constexpr size_t required_alignment = alignof(T *);

    // ------------------------------
    // Struct creation
    // ------------------------------

    explicit atomic_ref(T *&ref) noexcept : base_type(ref) {}
    atomic_ref(const atomic_ref &ref) noexcept : base_type(ref) {}

    using base_type::operator=;

    // ------------------------------
    // Pointer arithmetic operations
    // ------------------------------

    NODISCARD FORCE_INLINE_F value_type
    fetch_add(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::fetch_add(ptr_, GetByteOffset_(arg), order);
    }

    NODISCARD FORCE_INLINE_F value_type
    fetch_sub(difference_type arg, memory_order order = memory_order_seq_cst) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::fetch_sub(ptr_, GetByteOffset_(arg), order);
    }

    // ------------------------------
    // Compound assignment operators
    // ------------------------------

    value_type operator+=(difference_type arg) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::add_fetch(
            ptr_, GetByteOffset_(arg), memory_order_seq_cst
        );
    }

    value_type operator-=(difference_type arg) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::sub_fetch(
            ptr_, GetByteOffset_(arg), memory_order_seq_cst
        );
    }

    // ------------------------------
    // Increment/decrement operators
    // ------------------------------

    value_type operator++(int) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return fetch_add(1);
    }

    value_type operator--(int) const noexcept
        requires(!std::is_const_v<T *>)
    {
        return fetch_sub(1);
    }

    value_type operator++() const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::add_fetch(ptr_, sizeof(T), memory_order_seq_cst);
    }

    value_type operator--() const noexcept
        requires(!std::is_const_v<T *>)
    {
        return internal::AtomicImpl<T *, true>::sub_fetch(ptr_, sizeof(T), memory_order_seq_cst);
    }

    private:
    static constexpr difference_type GetByteOffset_(difference_type arg) { return arg * sizeof(T); }

    using base_type::ptr_;
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_ATOMIC_HPP_
