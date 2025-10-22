#ifndef ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_

#include "extensions/bits_ext.hpp"
#include "mem/heap.hpp"

namespace alloca
{
using namespace Mem;

template <class T, size_t kAlign = alignof(T)>
class DynArray
{
    static_assert(alignof(T) <= kAlign);
    static_assert(kAlign % alignof(T) == 0);

    public:
    /* type usings */
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = T *;
    using const_pointer   = const T *;

    /* iterators */
    using iterator               = value_type *;
    using const_iterator         = const value_type *;
    using reverse_iterator       = iterator;
    using const_reverse_iterator = const_iterator;

    // ------------------------------
    // Class creation
    // ------------------------------

    DynArray() = default;
    explicit DynArray(const size_t size) : mem_(nullptr) { Reallocate(size); }
    ~DynArray()
    {
        if (mem_) {
            KFree(mem_);
        }
    }

    // ------------------------------
    // Iterators
    // ------------------------------

    NODISCARD FORCE_INLINE_F iterator begin() noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F const_iterator begin() const noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F const_iterator cbegin() const noexcept { return mem_; }

    NODISCARD FORCE_INLINE_F iterator end() noexcept { return mem_ + size(); }

    NODISCARD FORCE_INLINE_F const_iterator end() const noexcept { return mem_ + size(); }

    NODISCARD FORCE_INLINE_F const_iterator cend() const noexcept { return mem_ + size(); }

    // ------------------------------
    // Class interaction
    // ------------------------------

    NODISCARD FORCE_INLINE_F T &operator[](const size_t index) noexcept
    {
        ASSERT_LT(index, size());
        return mem_[index];
    }

    NODISCARD FORCE_INLINE_F const T &operator[](const size_t index) const noexcept
    {
        ASSERT_LT(index, size());
        return mem_[index];
    }

    NODISCARD FORCE_INLINE_F T *data() { return mem_; }
    NODISCARD FORCE_INLINE_F const T *data() const { return mem_; }

    NODISCARD FORCE_INLINE_F size_t size() const { return size_; }
    NODISCARD FORCE_INLINE_F bool empty() const { return size_ == 0; }

    FORCE_INLINE_F Expected<void, MemError> Reallocate(const size_t size)
    {
        if (mem_) {
            AlignedKFree(mem_);
            KFree(mem_);
        }
        auto alloc = KMalloc({.size = size * sizeof(T), .alignment = kAlign});
        UNEXPETED_RET_IF_ERR(alloc);

        mem_  = static_cast<T *>(alloc.value());
        size_ = size;

        return {};
    }

    template <class... Args>
    FORCE_INLINE_F void AllocEntry(const size_t idx, Args &&...args)
    {
        ASSERT_LT(idx, size());
        T *data = mem_ + idx;
        new (reinterpret_cast<void *>(data)) T(std::forward<Args>(args)...);
    }

    FORCE_INLINE_F void FreeEntry(const size_t idx)
    {
        ASSERT_LT(idx, size());
        T *data = mem_ + idx;
        data->~T();
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    T *mem_{};
    size_t size_{};
};
}  // namespace alloca

#endif  // ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
