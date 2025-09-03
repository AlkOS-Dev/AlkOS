#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include <extensions/bit.hpp>

#include "extensions/bit_array.hpp"
#include "mem/physical_ptr.hpp"

#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

class PhysicalMemoryManager
{
    static constexpr u64 kPageSizeBytes = 1 < 12;

    void Init(Multiboot::MemoryMap memory_map, u64 lowest_safe_addr)
    {
        using namespace Multiboot;

        // Calculate bitmap size
        size_t avaliable_memory_bytes = 0;
        for (MmapEntry entry : memory_map) {
            if (entry.type == MmapEntry::kMemoryAvailable)
                avaliable_memory_bytes += entry.len;
        }

        // Round up division TODO Add to libc
        const size_t total_pages = (avaliable_memory_bytes + kPageSizeBytes - 1) / kPageSizeBytes;
        const size_t bitmap_size_bytes = (total_pages + 7) / 8;

        // Find place for bitmap
        u64 bitmap_addr = 0;
        for (MmapEntry entry : memory_map) {
            // TODO rework
            if (entry.addr < lowest_safe_addr) {
                continue;
            }

            if (entry.len < bitmap_size_bytes) {
                continue;
            }

            bitmap_addr = entry.len + entry.addr - bitmap_size_bytes;
            bitmap_addr = AlignDown(bitmap_addr, kPageSizeBytes);

            ASSERT(bitmap_addr + bitmap_size_bytes <= entry.addr + entry.len);
            break;
        }

        bitmap_ = BitMapView{reinterpret_cast<void *>(bitmap_addr), total_pages};

        // Reserve itself
        Reserve(PhysicalPtr<void>{bitmap_addr}, bitmap_size_bytes);
    }

    void Reserve(PhysicalPtr<void> addr, u64 size_bytes)
    {
        for (u64 i = 0; i < size_bytes; i += kPageSizeBytes) {
            Reserve(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Reserve(PhysicalPtr<void> addr)
    {
        ASSERT(IsAligned(addr.Value(), kPageSizeBytes));
        bitmap_.SetTrue(PageIndex(addr));
    }

    void Free(PhysicalPtr<void> addr)
    {
        ASSERT(IsAligned(addr.Value(), kPageSizeBytes));
        bitmap_.SetFalse(PageIndex(addr));
    }

    FORCE_INLINE_F size_t PageIndex(PhysicalPtr<void> addr)
    {
        return addr.Value() / kPageSizeBytes;
    }

    PhysicalPtr<void> Alloc() { return {}; }

    private:
    BitMapView bitmap_;
};

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_
