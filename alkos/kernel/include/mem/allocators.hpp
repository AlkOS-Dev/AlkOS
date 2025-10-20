#ifndef ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_

#include "mem/heap.hpp"

namespace alloca
{

mem::Expected<mem::VirtualPtr<void>, mem::MemError> AlignedKMalloc(size_t size, size_t alignment);
FAST_CALL mem::Expected<void, mem::MemError> AlignedKFree(void*) { return {}; }

template <class T, size_t kAlign>
class DynArray
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    DynArray() = default;
    explicit DynArray(const size_t size) : size_(size)
    {
        ASSERT_LE(alignof(T), kAlign);
        auto alloc = mem::KMalloc(kAlign + size * sizeof(T));  // safety buffer
        ASSERT_TRUE(static_cast<bool>(alloc));
    }

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    T* mem_{};
    size_t size_{};
};
}  // namespace alloca

#endif  // ALKOS_KERNEL_INCLUDE_MEM_ALLOCATORS_HPP_
