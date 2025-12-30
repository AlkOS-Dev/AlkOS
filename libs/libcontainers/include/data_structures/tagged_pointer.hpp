#ifndef LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_
#define LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_

#include <algorithm.hpp>
#include <defines.hpp>
#include <mem/heap.hpp>
#include <new.hpp>
#include <type_traits.hpp>
#include <utility.hpp>

namespace data_structures
{
/**
 * @brief A tagged pointer that stores type information in unused pointer bits.
 *
 * The number of available tag bits is determined by the number of types provided.
 *
 * @tparam TaggedTypes Variadic list of types that can be stored with this tagged pointer
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
    NODISCARD static TaggedPointer FromPtr(T *ptr)
    {
        static_assert(
            (std::is_same_v<T *, TaggedTypes> || ...), "Type T must be one of the TaggedTypes"
        );
        return TaggedPointer(TagPtr(ptr, GetTypeIndex<T>()));
    }

    ~TaggedPointer() { Destroy(); }

    TaggedPointer(const TaggedPointer &)            = delete;
    TaggedPointer &operator=(const TaggedPointer &) = delete;

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
            (std::is_same_v<T, TaggedTypes> || ...), "Type T must be one of the TaggedTypes"
        );

        auto mem = Mem::KMallocAligned(
            {.size      = sizeof(T),
             .alignment = alignof(T) > (1ULL << kTagBits) ? alignof(T) : (1ULL << kTagBits)}
        );
        if (!mem) {
            return {};
        }

        T *ptr = new (*mem) T(std::forward<Args>(args)...);

        return TaggedPointer(TagPtr(ptr, GetTypeIndex<T>()));
    }

    template <typename T>
    NODISCARD FORCE_INLINE_F bool Is() const
    {
        static_assert(
            (std::is_same_v<T, TaggedTypes> || ...), "Type T must be one of the TaggedTypes"
        );
        return IsValid() && GetTag() == GetTypeIndex<T>();
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

        // Dispatch to the correct type
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
    static constexpr size_t GetTypeIndex()
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
    void DestroyType()
    {
        if constexpr (!std::is_pointer_v<T>) {
            T *ptr = static_cast<T *>(UntagPtr());
            ptr->~T();
            Mem::KFreeAligned(ptr);
        }
    }

    template <typename Visitor, typename T, typename... Rest>
    decltype(auto) VisitImpl(Visitor &&visitor, size_t tag, size_t &current_index)
    {
        if (current_index == tag) {
            return visitor(As<T>());
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
            return visitor(As<T>());
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

}  // namespace data_structures

#endif  // LIBS_LIBCONTAINERS_INCLUDE_DATA_STRUCTURES_TAGGED_POINTER_HPP_
