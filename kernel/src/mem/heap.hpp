#ifndef KERNEL_SRC_MEM_HEAP_HPP_
#define KERNEL_SRC_MEM_HEAP_HPP_

#include <expected.hpp>
#include <new.hpp>
#include <utility.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"

namespace Mem
{

using std::expected;
using std::unexpected;

class PageMetaTable;
class BuddyPmm;
class SlabAllocator;

// ---------------------------------------------------------------------------
// Aligned Heap API types
// ---------------------------------------------------------------------------

struct KMallocRequest {
    size_t size;
    size_t alignment;
};

// ---------------------------------------------------------------------------
// Heap Class
// ---------------------------------------------------------------------------

class Heap
{
    public:
    void Init(PageMetaTable &pmt, BuddyPmm &pmm, SlabAllocator &slab);

    expected<VPtr<void>, MemError> Malloc(size_t size);
    void Free(VPtr<void> ptr);

    expected<VPtr<void>, MemError> MallocAligned(KMallocRequest request);
    void FreeAligned(VPtr<void> ptr);

    private:
    PageMetaTable *pmt_  = nullptr;
    BuddyPmm *pmm_       = nullptr;
    SlabAllocator *slab_ = nullptr;
};

// ---------------------------------------------------------------------------
// Standard Heap API
// Low overhead. Returns memory naturally aligned by the allocator
// (usually 8 or 16 bytes depending on size).
// ---------------------------------------------------------------------------

expected<VPtr<void>, MemError> KMalloc(size_t size);

template <typename T>
expected<VPtr<T>, MemError> KMalloc()
{
    return KMalloc(sizeof(T)).transform([](VPtr<void> ptr) {
        return reinterpret_cast<VPtr<T>>(ptr);
    });
}

template <typename T>
void KFree(VPtr<T> ptr);

// ---------------------------------------------------------------------------
// Aligned Heap API
// Higher overhead. Guarantees specific alignment.
// pointer returned MUST be freed using KFreeAligned.
// ---------------------------------------------------------------------------

expected<VPtr<void>, MemError> KMallocAligned(KMallocRequest request);

template <typename T>
void KFreeAligned(VPtr<T> ptr);

// ---------------------------------------------------------------------------
// Template Implementations
// ---------------------------------------------------------------------------

template <typename T>
void KFree(VPtr<T> ptr)
{
    KFree(reinterpret_cast<VPtr<void>>(ptr));
}

template <>
void KFree(VPtr<void> ptr);

template <typename T>
void KFreeAligned(VPtr<T> ptr)
{
    KFreeAligned(reinterpret_cast<VPtr<void>>(ptr));
}

template <>
void KFreeAligned(VPtr<void> ptr);

// ---------------------------------------------------------------------------
// Object Lifecycle API (new/delete equivalents)
// ---------------------------------------------------------------------------

/// KNew: Allocates memory and constructs the object T
template <typename T, typename... Args>
expected<VPtr<T>, MemError> KNew(Args &&...args)
{
    return KMalloc(sizeof(T)).transform([&](VPtr<void> ptr) {
        new (reinterpret_cast<void *>(ptr)) T(std::forward<Args>(args)...);
        return reinterpret_cast<VPtr<T>>(ptr);
    });
}

/// KDelete: Destructs the object T and frees memory
template <typename T>
void KDelete(VPtr<T> ptr)
{
    if (!ptr) {
        return;
    }
    reinterpret_cast<T *>(ptr)->~T();
    KFree(reinterpret_cast<VPtr<void>>(ptr));
}

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_HEAP_HPP_
