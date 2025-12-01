#include "mem/heap.hpp"

#include "hal/constants.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/phys/mngr/slab.hpp"
#include "modules/memory.hpp"

namespace Mem
{

// Header used ONLY by Aligned functions to store the original pointer
struct AllocationHeader {
    VPtr<void> original_ptr;
};

// ---------------------------------------------------------------------------
// Heap Class Implementation
// ---------------------------------------------------------------------------

void Heap::Init(PageMetaTable &pmt, BuddyPmm &pmm, SlabAllocator &slab)
{
    pmt_  = &pmt;
    pmm_  = &pmm;
    slab_ = &slab;
}

Expected<VPtr<void>, MemError> Heap::Malloc(size_t size)
{
    if (size == 0) {
        return Unexpected(MemError::InvalidArgument);
    }

    // Small Allocations -> Slab Allocator
    if (size < hal::kPageSizeBytes) {
        auto *cache = slab_->GetCache(size);
        if (!cache) {
            // Size is small but no cache fits? Likely a bug
            FAIL_ALWAYS("KMalloc called with small size but no cache fits");
        }

        auto res = cache->Alloc();
        UNEXPECTED_RET_IF_ERR(res);
        return *res;
    }

    // Large Allocations -> Buddy Allocator
    auto res = pmm_->Alloc({.order = BuddyPmm::SizeToPageOrder(size)});
    UNEXPECTED_RET_IF_ERR(res);

    return PhysToVirt(*res);
}

void Heap::Free(VPtr<void> ptr)
{
    if (ptr == nullptr)
        return;

    // Look up page metadata to determine who owns this pointer
    PPtr<Page> pptr = reinterpret_cast<PPtr<Page>>(VirtToPhys(ptr));
    // Align down to page size to get the Page struct start for metadata lookup
    auto &page_meta = pmt_->GetPageMeta(AlignDown(pptr, hal::kPageSizeBytes));

    if (page_meta.type == PageMetaType::Slab) {
        SlabMeta &sm = PageMeta::AsSlab(page_meta);
        sm.cache->Free(ptr);
        return;
    }

    if (page_meta.type == PageMetaType::Allocated) {
        pmm_->Free(pptr);
        return;
    }

    FAIL_ALWAYS("KFree called on pointer with corrupted or invalid PageMetaType");
}

Expected<VPtr<void>, MemError> Heap::MallocAligned(KMallocRequest r)
{
    size_t alignment = r.alignment;
    size_t size      = r.size;

    // Sanity checks
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        // Alignment must be power of 2
        return Unexpected(MemError::InvalidArgument);
    }

    size_t alloc_size = size + alignment + sizeof(AllocationHeader);

    auto res = Malloc(alloc_size);
    UNEXPECTED_RET_IF_ERR(res);

    VPtr<void> raw_ptr = *res;
    uptr raw_addr      = PtrToUptr(raw_ptr);

    // Calculate aligned address
    // We reserve space for header right before the aligned address
    uptr header_end   = raw_addr + sizeof(AllocationHeader);
    uptr aligned_addr = AlignUp(header_end, alignment);

    // Store the header immediately preceding the aligned address
    auto *header = reinterpret_cast<AllocationHeader *>(aligned_addr - sizeof(AllocationHeader));
    header->original_ptr = raw_ptr;

    return reinterpret_cast<VPtr<void>>(aligned_addr);
}

void Heap::FreeAligned(VPtr<void> ptr)
{
    if (ptr == nullptr)
        return;

    // Step back to retrieve the header
    uptr aligned_addr = PtrToUptr(ptr);
    auto *header = reinterpret_cast<AllocationHeader *>(aligned_addr - sizeof(AllocationHeader));

    VPtr<void> raw_ptr = header->original_ptr;
    Free(raw_ptr);
}

// ---------------------------------------------------------------------------
// Standard Allocator Implementation Wrappers
// ---------------------------------------------------------------------------

Expected<VPtr<void>, MemError> KMalloc(size_t size)
{
    return MemoryModule::Get().GetHeap().Malloc(size);
}

template <>
void KFree(VPtr<void> ptr)
{
    MemoryModule::Get().GetHeap().Free(ptr);
}

// ---------------------------------------------------------------------------
// Aligned Allocator Implementation Wrappers
// ---------------------------------------------------------------------------

Expected<VPtr<void>, MemError> KMallocAligned(KMallocRequest r)
{
    return MemoryModule::Get().GetHeap().MallocAligned(r);
}

template <>
void KFreeAligned(VPtr<void> ptr)
{
    MemoryModule::Get().GetHeap().FreeAligned(ptr);
}

}  // namespace Mem
