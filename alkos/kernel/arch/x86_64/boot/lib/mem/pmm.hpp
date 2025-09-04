#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include <extensions/algorithm.hpp>
#include <extensions/bit.hpp>
#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>
#include <extensions/internal/formats.hpp>
#include <extensions/optional.hpp>

#include <extensions/debug.hpp>

#include "mem/error.hpp"
#include "mem/physical_ptr.hpp"

#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

class PhysicalMemoryManager
{
    // TODO
    // Should add overflow checked arithmetic to libc and use it here

    static constexpr u64 kPageSize = 1 << 12;

    enum {
        BitMapFree     = 0,
        BitMapReserved = 1,
    };

    public:
    std::expected<void, MemError> Init(Multiboot::MemoryMap mem_map, u64 lowest_safe_addr);

    void Reserve(PhysicalPtr<void> addr, u64 size);

    void Reserve(PhysicalPtr<void> addr);

    void Free(PhysicalPtr<void> addr, u64 size);

    void Free(PhysicalPtr<void> addr);

    std::expected<PhysicalPtr<void>, MemError> Alloc();
    std::expected<PhysicalPtr<void>, MemError> Alloc32();
    std::expected<PhysicalPtr<void>, MemError> AllocContiguous(u64 size);
    std::expected<PhysicalPtr<void>, MemError> AllocContiguous32(u64 size);

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    FORCE_INLINE_F size_t PageIndex(PhysicalPtr<void> addr) { return addr.Value() / kPageSize; }

    std::expected<void, MemError> IterateToNextFreePage();
    std::expected<void, MemError> IterateToNextFreePage32();

    // Init Helpers

    std::tuple<u64, u64> CalcBitmapSize(Multiboot::MemoryMap& mem_map);

    std::expected<u64, MemError> FindBitmapLocation(
        Multiboot::MemoryMap mem_map, u64 bitmap_size, u64 lowest_safe_addr
    );

    void InitBitmapView(const u64 addr, const u64 size);

    void InitFreeMemory(Multiboot::MemoryMap& mem_map);

    void InitializeIterationIndices(Multiboot::MemoryMap& mem_map);

    //==============================================================================
    // Private fields
    //==============================================================================

    BitMapView* bitmap_view_{nullptr};
    alignas(64) byte bitmap_view_storage_[sizeof(BitMapView)]{};
    bool bitmap_view_initialized_{false};

    size_t iteration_index_{0};
    size_t iteration_index_32_{0};  // Used for 32-bit allocations
};

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_