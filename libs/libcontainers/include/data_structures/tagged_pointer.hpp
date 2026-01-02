#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_

#include <defines.hpp>
#include <mem/heap.hpp>
#include <new.hpp>
#include <type_traits.hpp>
#include <utility.hpp>
#include "ref_count.hpp"

namespace data_structures
{

template <typename T>
struct Owned {
    using Type                   = T;
    static constexpr bool kOwned = true;
};

template <typename T>
struct NonOwned {
    using Type                   = T;
    static constexpr bool kOwned = false;
};

namespace internal
{

template <typename T>
struct UnwrapType {
    using Type = T;
};

template <typename T>
struct UnwrapType<Owned<T>> {
    using Type = T;
};

template <typename T>
struct UnwrapType<NonOwned<T>> {
    using Type = T;
};

template <typename T>
using UnwrapType_t = typename UnwrapType<T>::Type;

template <typename T>
struct IsOwned : std::false_type {
};

template <typename T>
struct IsOwned<Owned<T>> : std::true_type {
};

template <typename T>
inline constexpr bool IsOwned_v = IsOwned<T>::value;

template <typename T>
inline constexpr bool IsRefCounted_v =
    std::derived_from<T, RefCounted<T, false>> || std::derived_from<T, RefCounted<T, true>>;

}  // namespace internal

/**
 * @brief A tagged pointer that stores type information in unused pointer bits.
 *
 * The number of available tag bits is determined by the number of types provided.
 * Types can be wrapped in Owned<T> or NonOwned<T> to control ownership semantics.
 *
 * @tparam TaggedTypes Variadic list of types (can be Owned<T> or NonOwned<T>)
 */
template <typename... TaggedTypes>
class TaggedPointer
{
    public:
    using BaseT = uintptr_t;

    static constexpr size_t kNumTypes = sizeof...(TaggedTypes);
    static constexpr size_t kTagBits  = []() constexpr {
        size_t bits = 0;
        while ((1ULL << bits) < kNumTypes) {
            bits++;
        }
        return bits;
    }();

    TaggedPointer() = default;

    explicit TaggedPointer(BaseT tagged_ptr) : tagged_ptr_(tagged_ptr) {}

    template <typename T>
    NODISCARD static TaggedPointer Wrap(T *ptr)
    {
        static_assert(
            (std::is_same_v<NonOwned<T>, TaggedTypes> || ...),
            "Type NonOwned<T> must be in the TaggedTypes list to use Wrap"
        );

        if constexpr (internal::IsRefCounted_v<T>) {
            if (ptr) {
                ptr->AddRef();
            }
        }

        return TaggedPointer(TagPtr(ptr, GetTypeIndex<NonOwned<T>>()));
    }

    ~TaggedPointer() { Destroy(); }

    // Copy constructor - only enabled if all types are non-owned
    TaggedPointer(const TaggedPointer &other)
        requires(!internal::IsOwned_v<TaggedTypes> && ...)
        : tagged_ptr_(other.tagged_ptr_)
    {
        if (IsValid()) {
            AddRefIfRefCounted();
        }
    }

    // Copy assignment - only enabled if all types are non-owned
    TaggedPointer &operator=(const TaggedPointer &other)
        requires(!internal::IsOwned_v<TaggedTypes> && ...)
    {
        if (this != &other) {
            Destroy();
            tagged_ptr_ = other.tagged_ptr_;
            if (IsValid()) {
                AddRefIfRefCounted();
            }
        }
        return *this;
    }

    TaggedPointer(TaggedPointer &&other) noexcept : tagged_ptr_(other.tagged_ptr_)
    {
        other.tagged_ptr_ = 0;
    }

    TaggedPointer &operator=(TaggedPointer &&other) noexcept
    {
        if (this != &other) {
            Destroy();
            tagged_ptr_       = other.tagged_ptr_;
            other.tagged_ptr_ = 0;
        }
        return *this;
    }

    NODISCARD FORCE_INLINE_F bool IsValid() const { return tagged_ptr_ != 0; }

    NODISCARD FORCE_INLINE_F explicit operator bool() const { return IsValid(); }

    template <typename T, typename... Args>
    NODISCARD FAST_CALL TaggedPointer Construct(Args &&...args)
    {
        static_assert(
            (std::is_same_v<Owned<T>, TaggedTypes> || ...),
            "Type Owned<T> must be in the TaggedTypes list to use Construct"
        );

        auto mem = Mem::KMallocAligned(
            {.size      = sizeof(T),
             .alignment = alignof(T) > (1ULL << kTagBits) ? alignof(T) : (1ULL << kTagBits)}
        );
        if (!mem) {
            return {};
        }

        T *ptr = new (*mem) T(std::forward<Args>(args)...);

        return TaggedPointer(TagPtr(ptr, GetTypeIndex<Owned<T>>()));
    }

    template <typename T>
    NODISCARD FORCE_INLINE_F bool Is() const
    {
        constexpr bool has_owned     = (std::is_same_v<Owned<T>, TaggedTypes> || ...);
        constexpr bool has_non_owned = (std::is_same_v<NonOwned<T>, TaggedTypes> || ...);
        static_assert(
            has_owned || has_non_owned, "Type T (as Owned<T> or NonOwned<T>) must be in TaggedTypes"
        );

        if (!IsValid())
            return false;

        size_t tag = GetTag();
        if constexpr (has_owned) {
            if (tag == GetTypeIndex<Owned<T>>())
                return true;
        }
        if constexpr (has_non_owned) {
            if (tag == GetTypeIndex<NonOwned<T>>())
                return true;
        }
        return false;
    }

    template <typename T>
    NODISCARD FORCE_INLINE_F T &As()
    {
        ASSERT_TRUE(Is<T>());
        return *static_cast<T *>(UntagPtr());
    }

    template <typename T>
    NODISCARD FORCE_INLINE_F const T &As() const
    {
        ASSERT_TRUE(Is<T>());
        return *static_cast<const T *>(UntagPtr());
    }

    // Visitor pattern to handle different types
    template <typename Visitor>
    FORCE_INLINE_F decltype(auto) Visit(Visitor &&visitor)
    {
        ASSERT_TRUE(IsValid());
        size_t tag           = GetTag();
        size_t current_index = 0;

        return VisitImpl<Visitor, TaggedTypes...>(
            std::forward<Visitor>(visitor), tag, current_index
        );
    }

    template <typename Visitor>
    FORCE_INLINE_F decltype(auto) Visit(Visitor &&visitor) const
    {
        ASSERT_TRUE(IsValid());
        size_t tag           = GetTag();
        size_t current_index = 0;

        return VisitImpl<Visitor, TaggedTypes...>(
            std::forward<Visitor>(visitor), tag, current_index
        );
    }

    NODISCARD FORCE_INLINE_F uintptr_t GetRaw() const { return tagged_ptr_; }

    NODISCARD FORCE_INLINE_F uintptr_t Release()
    {
        uintptr_t tmp = tagged_ptr_;
        tagged_ptr_   = 0;
        return tmp;
    }

    private:
    FORCE_INLINE_F void Destroy()
    {
        if (IsValid()) {
            size_t tag           = GetTag();
            size_t current_index = 0;

            (void)((current_index++ == tag ? (DestroyType<TaggedTypes>(), true) : false) || ...);
        }
    }

    template <typename T>
    FORCE_INLINE_F static constexpr size_t GetTypeIndex()
    {
        size_t index = 0;
        ((std::is_same_v<T, TaggedTypes> ? true : (++index, false)) || ...);
        return index;
    }

    NODISCARD FORCE_INLINE_F static BaseT TagPtr(void *ptr, size_t type_index)
    {
        return reinterpret_cast<BaseT>(ptr) | type_index;
    }

    NODISCARD FORCE_INLINE_F void *UntagPtr() const
    {
        return reinterpret_cast<void *>(tagged_ptr_ & kPtrMask);
    }

    NODISCARD FORCE_INLINE_F size_t GetTag() const { return tagged_ptr_ & kTagMask; }

    template <typename T>
    FORCE_INLINE_F void DestroyType()
    {
        using ActualType = internal::UnwrapType_t<T>;
        ActualType *ptr  = static_cast<ActualType *>(UntagPtr());

        if (!ptr)
            return;

        // If owned, destroy and free memory
        if constexpr (internal::IsOwned_v<T>) {
            ptr->~ActualType();
            Mem::KFreeAligned(ptr);
        } else if constexpr (internal::IsRefCounted_v<ActualType>) {
            ptr->Release();
        }
    }

    FORCE_INLINE_F void AddRefIfRefCounted()
    {
        if (!IsValid())
            return;

        size_t tag           = GetTag();
        size_t current_index = 0;

        (void)((current_index++ == tag ? (AddRefType<TaggedTypes>(), true) : false) || ...);
    }

    template <typename T>
    FORCE_INLINE_F void AddRefType()
    {
        using ActualType = internal::UnwrapType_t<T>;
        ActualType *ptr  = static_cast<ActualType *>(UntagPtr());

        if (!ptr)
            return;

        if constexpr (internal::IsRefCounted_v<ActualType>) {
            ptr->AddRef();
        }
    }

    template <typename Visitor, typename T, typename... Rest>
    decltype(auto) VisitImpl(Visitor &&visitor, size_t tag, size_t &current_index)
    {
        if (current_index == tag) {
            using ActualType = internal::UnwrapType_t<T>;
            return visitor(*static_cast<ActualType *>(UntagPtr()));
        }
        current_index++;
        if constexpr (sizeof...(Rest) > 0) {
            return VisitImpl<Visitor, Rest...>(std::forward<Visitor>(visitor), tag, current_index);
        }
        R_FAIL_ALWAYS("Invalid tag in TaggedPointer::Visit");
    }

    template <typename Visitor, typename T, typename... Rest>
    decltype(auto) VisitImpl(Visitor &&visitor, size_t tag, size_t &current_index) const
    {
        if (current_index == tag) {
            using ActualType = internal::UnwrapType_t<T>;
            return visitor(*static_cast<const ActualType *>(UntagPtr()));
        }
        current_index++;
        if constexpr (sizeof...(Rest) > 0) {
            return VisitImpl<Visitor, Rest...>(std::forward<Visitor>(visitor), tag, current_index);
        }
        R_FAIL_ALWAYS("Invalid tag in TaggedPointer::Visit");
    }

    static constexpr BaseT kTagMask = (static_cast<BaseT>(1) << kTagBits) - 1;
    static constexpr BaseT kPtrMask = ~kTagMask;

    BaseT tagged_ptr_{};
};

template <typename... TaggedTypes>
using OwningTaggedPtr = TaggedPointer<Owned<TaggedTypes>...>;

template <typename... TaggedTypes>
using NonOwningTaggedPtr = TaggedPointer<NonOwned<TaggedTypes>...>;

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_
