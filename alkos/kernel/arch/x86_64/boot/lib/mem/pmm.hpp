#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include <extensions/bit.hpp>
#include <extensions/bit_array.hpp>
#include <extensions/expected.hpp>
#include <extensions/internal/formats.hpp>

#include <extensions/debug.hpp>

#include "mem/error.hpp"
#include "mem/physical_ptr.hpp"

#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

class PhysicalMemoryManager
{
    // TODO
    // Should add overflow checked arithmetic to libc and use it here
    //
    static constexpr u64 kPageSizeBytes = 1 << 12;

    enum {
        BitMapFree     = 0,
        BitMapReserved = 1,
    };

    public:
    std::expected<void, MemError> Init(Multiboot::MemoryMap memory_map, u64 lowest_safe_addr)
    {
        using namespace Multiboot;
        TRACE_DEBUG("Initializing Physical Memory Manager...");

        ASSERT_TRUE(!bitmap_view_initialized_, "PMM already initialized");

        // Calculate bitmap size
        u64 maximum_physical_address = 0;
        for (MmapEntry& entry : memory_map) {
            maximum_physical_address = std::max(maximum_physical_address, entry.addr + entry.len);
        }

        // Round up division TODO Add to libc
        const size_t total_pages =
            static_cast<u64>(maximum_physical_address + kPageSizeBytes - 1) / kPageSizeBytes;
        TRACE_DEBUG(
            "Total memory size: %sB, total pages: %llu", FormatMetricUint(maximum_physical_address),
            total_pages
        );
        const u64 bitmap_size_bytes = (total_pages + 7) / 8;

        // Find place for bitmap
        static constexpr u64 kInvalidBitmapAddr = static_cast<u64>(-1);
        u64 bitmap_addr                         = kInvalidBitmapAddr;
        for (MmapEntry& entry : memory_map) {
            // Calculate the maximum possible start address for the bitmap within this entry
            // ensuring it respects page alignment and fits within the entry.
            u64 potential_bitmap_start =
                AlignDown(entry.addr + entry.len - bitmap_size_bytes, kPageSizeBytes);

            if (potential_bitmap_start < entry.addr ||
                potential_bitmap_start + bitmap_size_bytes > entry.addr + entry.len) {
                continue;  // Bitmap does not fit or crosses start boundary
            }

            if (potential_bitmap_start < lowest_safe_addr) {
                continue;
            }

            bitmap_addr = potential_bitmap_start;
            break;
        }
        if (bitmap_addr == kInvalidBitmapAddr) {
            return std::unexpected{MemError::OutOfMemory};
        }

        // Initialize bitmap
        bitmap_view_ = new (bitmap_view_storage_)
            BitMapView{reinterpret_cast<void*>(bitmap_addr), total_pages};
        bitmap_view_->SetAll(BitMapReserved);
        bitmap_view_initialized_ = true;

        // Free available memory
        for (MmapEntry& entry : memory_map) {
            if (entry.type == MmapEntry::kMemoryAvailable) {
                u64 start_addr = entry.addr;
                u64 end_addr   = entry.addr + entry.len;

                Free(PhysicalPtr<void>{start_addr}, end_addr - start_addr);
            }
        }

        // Reserve itself
        Reserve(PhysicalPtr<void>{bitmap_addr}, bitmap_size_bytes);

        ResetIterationIndex();

        return {};
    }

    void Reserve(PhysicalPtr<void> addr, u64 size_bytes)
    {
        for (u64 i = 0; i < AlignUp(size_bytes, kPageSizeBytes); i += kPageSizeBytes) {
            Reserve(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Reserve(PhysicalPtr<void> addr)
    {
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSizeBytes));
        ASSERT_TRUE(!bitmap_view_->Get(PageIndex(addr)), "Reserving already reserved page");
        bitmap_view_->SetTrue(PageIndex(addr));
    }

    void Free(PhysicalPtr<void> addr, u64 size_bytes)
    {
        for (u64 i = 0; i < AlignUp(size_bytes, kPageSizeBytes); i += kPageSizeBytes) {
            Free(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Free(PhysicalPtr<void> addr)
    {
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSizeBytes));
        ASSERT_TRUE(bitmap_view_->Get(PageIndex(addr)), "Freeing already free page");
        bitmap_view_->SetFalse(PageIndex(addr));
    }

    // TODO: Alloc contiguous memory

    std::expected<PhysicalPtr<void>, MemError> Alloc()
    {
        auto res = FindNextFreePage();
        if (!res) {
            return std::unexpected{res.error()};
        }

        ASSERT_TRUE(!bitmap_view_->Get(iteration_index_), "Allocating already allocated page");
        bitmap_view_->SetTrue(iteration_index_);
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

    FORCE_INLINE_F void ResetIterationIndex() { iteration_index_ = bitmap_view_->Size() - 1; }

    FORCE_INLINE_F std::expected<void, MemError> FindNextFreePage()
    {
        const size_t total_pages = bitmap_view_->Size();
        for (size_t i = 0; i < total_pages; i++) {
            // Reverse iteration to alloc higher addresses first
            const size_t idx = (iteration_index_ - i) % total_pages;
            if (!bitmap_view_->Get(idx)) {
                iteration_index_ = idx;
                return {};
            }
        }

        return std::unexpected{MemError::OutOfMemory};
    }

    //==============================================================================
    // Private fields
    //==============================================================================

    BitMapView* bitmap_view_{nullptr};
    alignas(64) byte bitmap_view_storage_[sizeof(BitMapView)]{};
    bool bitmap_view_initialized_{false};

    size_t iteration_index_{0};
};

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_
