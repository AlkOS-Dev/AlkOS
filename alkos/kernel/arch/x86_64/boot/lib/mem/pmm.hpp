#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include <extensions/bit.hpp>
#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/physical_ptr.hpp"

#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

class PhysicalMemoryManager
{
    static constexpr u64 kPageSizeBytes = 1 < 12;

    std::expected<void, MemError> Init(Multiboot::MemoryMap memory_map, u64 lowest_safe_addr)
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
        static constexpr u64 kInvalidBitmapAddr = static_cast<u64>(-1);
        u64 bitmap_addr                         = kInvalidBitmapAddr;
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

            ASSERT_LE(bitmap_addr, entry.addr + entry.len - bitmap_size_bytes);
            break;
        }
        if (bitmap_addr == kInvalidBitmapAddr) {
            return std::unexpected{MemError::OutOfMemory};
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
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSizeBytes));
        ASSERT_TRUE(!bitmap_.Get(PageIndex(addr)), "Reserving already reserved page");
        bitmap_.SetTrue(PageIndex(addr));
    }

    void Free(PhysicalPtr<void> addr, u64 size_bytes)
    {
        for (u64 i = 0; i < size_bytes; i += kPageSizeBytes) {
            Free(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Free(PhysicalPtr<void> addr)
    {
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSizeBytes));
        ASSERT_TRUE(bitmap_.Get(PageIndex(addr)), "Freeing already free page");
        bitmap_.SetFalse(PageIndex(addr));
    }

    // TODO: Alloc contiguous memory

    std::expected<PhysicalPtr<void>, MemError> Alloc()
    {
        auto res = FindNextFreePage();
        if (!res) {
            return std::unexpected{res.error()};
        }

        ASSERT_TRUE(!bitmap_.Get(iteration_index_), "Allocating already allocated page");
        bitmap_.SetTrue(iteration_index_);
        return PhysicalPtr<void>{iteration_index_ * kPageSizeBytes};
    }

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    FORCE_INLINE_F size_t PageIndex(PhysicalPtr<void> addr)
    {
        return addr.Value() / kPageSizeBytes;
    }

    FORCE_INLINE_F std::expected<void, MemError> FindNextFreePage()
    {
        const size_t total_pages = bitmap_.Size();
        for (size_t i = 0; i < total_pages; i++) {
            const size_t idx = (iteration_index_ + i) % total_pages;
            if (!bitmap_.Get(idx)) {
                iteration_index_ = idx;
                return {};
            }
        }

        return std::unexpected{MemError::OutOfMemory};
    }

    //==============================================================================
    // Private fields
    //==============================================================================

    BitMapView bitmap_;
    size_t iteration_index_{0};
};

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_
