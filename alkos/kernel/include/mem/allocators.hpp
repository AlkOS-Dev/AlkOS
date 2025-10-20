#ifndef ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_

#include "extensions/bits_ext.hpp"
#include "mem/heap.hpp"

namespace alloca
{
using namespace mem;
FAST_CALL Expected<VirtualPtr<void>, MemError> AlignedKMalloc(
    const size_t size, const size_t alignment
)
{
    ASSERT_TRUE(IsPowerOfTwo(alignment));
    const size_t full_size = size + alignment - 1 + sizeof(void*);
    auto result            = KMalloc(full_size);
    if (!result) {
        return result;
    }

    const u64 addr    = reinterpret_cast<u64>(result.value()) + sizeof(void*);
    const u64 aligned = (addr + alignment - 1) & ~(alignment - 1);

    *reinterpret_cast<void**>(aligned - sizeof(void*)) = result.value();
    return reinterpret_cast<void*>(aligned);
}

FAST_CALL Expected<void, MemError> AlignedKFree(void* ptr)
{
    if (!ptr)
        return {};
    void* original = (static_cast<void**>(ptr))[-1];
    KFree(original);
}

template <class T, size_t kAlign>
class DynArray
{
    static_assert(alignof(T) <= kAlign);
    static_assert(kAlign % alignof(T) == 0);

    public:
    /* type usings */
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = T*;
    using const_pointer   = const T*;

    /* iterators */
    using iterator               = value_type*;
    using const_iterator         = const value_type*;
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
            AlignedFree(mem_);
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

    NODISCARD FORCE_INLINE_F T& operator[](const size_t index) noexcept
    {
        ASSERT_LE(index, size());
        return mem_[index];
    }

    NODISCARD FORCE_INLINE_F const T& operator[](const size_t index) const noexcept
    {
        ASSERT_LE(index, size());
        return mem_[index];
    }

    NODISCARD FORCE_INLINE_F T* data() { return mem_; }
    NODISCARD FORCE_INLINE_F const T* data() const { return mem_; }

    NODISCARD FORCE_INLINE_F size_t size() const { return size_; }
    NODISCARD FORCE_INLINE_F bool empty() const { return size_ == 0; }

    FORCE_INLINE_F Expected<void, MemError> Reallocate(const size_t size)
    {
        if (mem_) {
            AlignedKFree(mem_);
        }
        auto alloc = AlignedKMalloc(size * sizeof(T), kAlign);

        if (!alloc) {
            return alloc;
        }

        mem_  = alloc.value();
        size_ = size;

        return {};
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    T* mem_{};
    size_t size_{};
};
}  // namespace alloca

#endif  // ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
