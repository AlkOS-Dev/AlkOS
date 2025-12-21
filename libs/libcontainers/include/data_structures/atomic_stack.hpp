#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_STACK_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_STACK_HPP_

#include <assert.h>
#include <defines.hpp>
#include <type_traits.hpp>
#include <utility.hpp>

#include "hal/sync.hpp"

template <size_t kSizeBytes, size_t kAlignment = 8>
class AtomicArrayStaticStack
{
    // ------------------------------
    // Class creation
    // ------------------------------

    template <class T>
    using clean_type = std::remove_reference_t<std::remove_const_t<T>>;

    public:
    AtomicArrayStaticStack()  = default;
    ~AtomicArrayStaticStack() = default;

    AtomicArrayStaticStack(const AtomicArrayStaticStack &) = default;
    AtomicArrayStaticStack(AtomicArrayStaticStack &&)      = default;

    AtomicArrayStaticStack &operator=(const AtomicArrayStaticStack &) = default;
    AtomicArrayStaticStack &operator=(AtomicArrayStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    template <class T>
    FORCE_INLINE_F T &Push(const T &item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(item); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F T &Push(T &&item)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::move(item)); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T, class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        auto ptr = PushMem_<T>();
        new (ptr) clean_type<T>(std::forward<Args>(args)...); /* Checks alignment on debug */
        return *ptr;
    }

    template <class T>
    FORCE_INLINE_F clean_type<T> Pop()
    {
        ASSERT_LE(sizeof(T), static_cast<size_t>(hal::AtomicLoad(&top_)), "Stack underflow!!");
        const auto top = hal::AtomicSub(&top_, sizeof(T));

        auto ptr = reinterpret_cast<clean_type<T> *>(stack_.data + top);
        return std::move(*ptr);
    }

    NODISCARD FORCE_INLINE_F size_t Size() const { return hal::AtomicLoad(&top_); }

    NODISCARD FORCE_INLINE_F const void *Data() const { return stack_.data; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    template <class T>
    clean_type<T> *PushMem_() noexcept
    {
        const size_t start = hal::AtomicLoad(&top_);
        hal::AtomicAdd(&top_, sizeof(T));

        ASSERT_LE(static_cast<size_t>(hal::AtomicLoad(&top_)), kSizeBytes, "Stack overflow!");
        return reinterpret_cast<clean_type<T> *>(stack_.data + start);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    hal::Atomic64 top_{};
    std::aligned_storage_t<kSizeBytes, kAlignment> stack_{};
};

template <class T, size_t kNumObjects>
class AtomicArraySingleTypeStaticStack
    : public AtomicArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>
{
    using base_t = AtomicArrayStaticStack<sizeof(T) * kNumObjects, alignof(T)>;
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    AtomicArraySingleTypeStaticStack()  = default;
    ~AtomicArraySingleTypeStaticStack() = default;

    AtomicArraySingleTypeStaticStack(const AtomicArraySingleTypeStaticStack &) = default;
    AtomicArraySingleTypeStaticStack(AtomicArraySingleTypeStaticStack &&)      = default;

    AtomicArraySingleTypeStaticStack &operator=(const AtomicArraySingleTypeStaticStack &) = default;
    AtomicArraySingleTypeStaticStack &operator=(AtomicArraySingleTypeStaticStack &&)      = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F T &Push(const T &item) { return base_t::Push(item); }

    FORCE_INLINE_F T &Push(T &&item) { return base_t::Push(std::move(item)); }

    template <class... Args>
    FORCE_INLINE_F T &PushEmplace(Args &&...args)
    {
        return base_t::template PushEmplace<T>(std::forward<Args>(args)...);
    }

    FORCE_INLINE_F T Pop() { return base_t::template Pop<T>(); }

    FORCE_INLINE_F size_t Size() const { return base_t::Size() / sizeof(T); }

    FORCE_INLINE_F size_t SizeBytes() const { return base_t::Size(); }

    using base_t::Data;
};

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_ATOMIC_STACK_HPP_
