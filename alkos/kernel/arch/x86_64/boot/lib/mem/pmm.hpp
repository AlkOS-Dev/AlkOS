#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

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
    //
    static constexpr u64 kPageSize = 1 << 12;

    enum {
        BitMapFree     = 0,
        BitMapReserved = 1,
    };

    public:
    std::expected<void, MemError> Init(Multiboot::MemoryMap mem_map, u64 lowest_safe_addr)
    {
        using namespace Multiboot;
        TRACE_DEBUG("Initializing Physical Memory Manager...");
        ASSERT_TRUE(!bitmap_view_initialized_, "PMM already initialized");

        // Calc size
        TRACE_DEBUG("Calculating bitmap size...");
        const auto [bitmap_size, total_pages] = CalcBitmapSize(mem_map);

        // Find location
        TRACE_DEBUG("Finding bitmap location...");
        const auto bitmap_addr_res = FindBitmapLocation(mem_map, bitmap_size, lowest_safe_addr);
        if (!bitmap_addr_res) {
            return std::unexpected(bitmap_addr_res.error());
        }
        const u64 bitmap_addr = bitmap_addr_res.value();

        // Initialize bitmap
        TRACE_DEBUG("Initializing bitmap view...");
        InitBitmapView(bitmap_addr, bitmap_size);

        // Free available memory
        TRACE_DEBUG("Initializing free memory...");
        InitFreeMemory(mem_map);

        // Reserve itself
        TRACE_DEBUG("Reserving bitmap memory...");
        Reserve(PhysicalPtr<void>{bitmap_addr}, bitmap_size);

        return {};
    }

    void Reserve(PhysicalPtr<void> addr, u64 size)
    {
        const u64 start_addr = AlignDown(addr.Value(), kPageSize);
        const u64 end_addr   = AlignUp(addr.Value() + size, kPageSize);
        for (u64 i = 0; i < end_addr - start_addr; i += kPageSize) {
            Reserve(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Reserve(PhysicalPtr<void> addr)
    {
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSize));
        ASSERT_TRUE(!bitmap_view_->Get(PageIndex(addr)), "Reserving already reserved page");
        bitmap_view_->SetTrue(PageIndex(addr));
    }

    void Free(PhysicalPtr<void> addr, u64 size)
    {
        const u64 start_addr = AlignDown(addr.Value(), kPageSize);
        const u64 end_addr   = AlignUp(addr.Value() + size, kPageSize);
        for (u64 i = 0; i < end_addr - start_addr; i += kPageSize) {
            Free(PhysicalPtr<void>{addr.Value() + i});
        }
    }

    void Free(PhysicalPtr<void> addr)
    {
        ASSERT_TRUE(IsAligned(addr.Value(), kPageSize));
        ASSERT_TRUE(bitmap_view_->Get(PageIndex(addr)), "Freeing already free page");
        bitmap_view_->SetFalse(PageIndex(addr));
    }

    // TODO: Alloc contiguous memory

    std::expected<PhysicalPtr<void>, MemError> Alloc()
    {
        auto res = IterateToNextFreePage();
        if (!res) {
            return std::unexpected{res.error()};
        }

        ASSERT_TRUE(!bitmap_view_->Get(iteration_index_), "Allocating already allocated page");
        bitmap_view_->SetTrue(iteration_index_);
        return PhysicalPtr<void>{iteration_index_ * kPageSize};
    }

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    FORCE_INLINE_F size_t PageIndex(PhysicalPtr<void> addr) { return addr.Value() / kPageSize; }

    FORCE_INLINE_F std::expected<void, MemError> IterateToNextFreePage()
    {
        const size_t total_pages = bitmap_view_->Size();
        size_t original_index    = iteration_index_;

        do {
            if (iteration_index_ == 0) {
                iteration_index_ = total_pages - 1;
            } else {
                --iteration_index_;
            }

            if (!bitmap_view_->Get(iteration_index_)) {
                return {};  // Found a free page
            }
        } while (iteration_index_ != original_index);

        return std::unexpected{MemError::OutOfMemory};
    }

    // Init Helpers

    std::tuple<u64, u64> CalcBitmapSize(Multiboot::MemoryMap& mem_map)
    {
        using namespace Multiboot;

        u64 maximum_physical_address = 0;
        for (MmapEntry& entry : mem_map) {
            maximum_physical_address = std::max(maximum_physical_address, entry.addr + entry.len);
        }

        // Round up division TODO Add to libc
        const size_t total_pages =
            static_cast<u64>(maximum_physical_address + kPageSize - 1) / kPageSize;
        TRACE_DEBUG(
            "Total memory size: %sB, total pages: %llu", FormatMetricUint(maximum_physical_address),
            total_pages
        );
        const u64 bitmap_size = (total_pages + 7) / 8;
        return {bitmap_size, total_pages};
    }

    std::expected<u64, MemError> FindBitmapLocation(
        Multiboot::MemoryMap mem_map, u64 bitmap_size, u64 lowest_safe_addr
    )
    {
        using namespace Multiboot;

        constexpr u64 kMax32BitAddress = 0xFFFFFFFF;

        std::optional<u64> bitmap_addr{};
        for (MmapEntry& entry : mem_map) {
            if (entry.type != MmapEntry::kMemoryAvailable) {
                continue;
            }

            if (entry.len < bitmap_size) {
                continue;
            }

            // Calculate the maximum possible start address for the bitmap within this entry
            // ensuring it respects page alignment and fits within the entry.
            u64 potential_bitmap_start = AlignDown(entry.addr + entry.len - bitmap_size, kPageSize);

            if (potential_bitmap_start < entry.addr ||
                potential_bitmap_start + bitmap_size > entry.addr + entry.len) {
                continue;  // Bitmap does not fit or crosses start boundary
            }

            if (potential_bitmap_start < lowest_safe_addr) {
                continue;
            }

            // Ensure the entire bitmap is within the 32-bit address range
            if (potential_bitmap_start + bitmap_size > kMax32BitAddress) {
                continue;
            }

            bitmap_addr.emplace(potential_bitmap_start);
            break;
        }
        if (!bitmap_addr.has_value()) {
            return std::unexpected{MemError::OutOfMemory};
        }

        return *bitmap_addr;
    }

    void InitBitmapView(const u64 addr, const u64 size)
    {
        TRACE_DEBUG("Bitmap located at: %#018llx, size: %llu bytes", addr, size);
        bitmap_view_ = new (bitmap_view_storage_) BitMapView{reinterpret_cast<void*>(addr), size};
        bitmap_view_->SetAll(BitMapReserved);
        bitmap_view_initialized_ = true;

        for (size_t i = 0; i < bitmap_view_->Size(); ++i) {
            ASSERT_TRUE(bitmap_view_->Get(i), "Bitmap not initialized correctly");
        }
    }

    void InitFreeMemory(Multiboot::MemoryMap& mem_map)
    {
        using namespace Multiboot;

        for (MmapEntry& entry : mem_map) {
            if (entry.type == MmapEntry::kMemoryAvailable) {
                u64 start_addr = AlignUp<u64>(entry.addr, kPageSize);
                u64 end_addr   = AlignDown<u64>(entry.addr + entry.len, kPageSize);

                Free(PhysicalPtr<void>{start_addr}, end_addr - start_addr);
            }
        }
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
