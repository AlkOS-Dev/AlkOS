#include "mem/pmm.hpp"

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

std::expected<PhysicalMemoryManager*, MemError> PhysicalMemoryManager::Create(
    Multiboot::MemoryMap mem_map, u64 lowest_safe_addr, byte* pmm_prealloc_mem
)
{
    using namespace Multiboot;

    TRACE_DEBUG("Initializing Physical Memory Manager...");

    TRACE_DEBUG("Calculating bitmap size...");
    const auto [bitmap_size, total_pages] = CalcBitmapSize(mem_map);

    TRACE_DEBUG("Finding bitmap location...");
    const auto bitmap_addr_res = FindBitmapLocation(mem_map, bitmap_size, lowest_safe_addr);
    if (!bitmap_addr_res) {
        return std::unexpected(bitmap_addr_res.error());
    }
    const u64 bitmap_addr = bitmap_addr_res.value();

    TRACE_DEBUG("Initializing bitmap view...");
    [[maybe_unused]] BitMapView bitmap_view = InitBitmapView(bitmap_addr, total_pages);

    TRACE_DEBUG("Creating PhysicalMemoryManager instance...");

    PhysicalMemoryManager* pmm_ptr = new (pmm_prealloc_mem) PhysicalMemoryManager(
        PmmState{
            .bitmap_addr        = bitmap_addr,
            .total_pages        = total_pages,
            .iteration_index    = 0,
            .iteration_index_32 = 0
        }
    );
    auto& pmm = *pmm_ptr;

    TRACE_DEBUG("Initializing free memory...");
    InitFreeMemory(pmm, mem_map);

    TRACE_DEBUG("Reserving bitmap memory...");
    pmm.Reserve(PhysicalPtr<void>{bitmap_addr}, bitmap_size);

    TRACE_DEBUG("Initializing iteration indices...");
    InitIterIndicies(pmm, mem_map);

    return pmm_ptr;
}

void PhysicalMemoryManager::Reserve(PhysicalPtr<void> addr, u64 size)
{
    const u64 start_addr = AlignDown(addr.Value(), kPageSize);
    const u64 end_addr   = AlignUp(addr.Value() + size, kPageSize);
    TRACE_DEBUG("Reserving memory: start=0x%llX, end=0x%llX", start_addr, end_addr);
    for (u64 i = 0; i < end_addr - start_addr; i += kPageSize) {
        Reserve(PhysicalPtr<void>{start_addr + i});
    }
}

void PhysicalMemoryManager::Reserve(PhysicalPtr<void> addr)
{
    ASSERT_TRUE(IsAligned(addr.Value(), kPageSize));
    if (BitMapFree != bitmap_view_.Get(PageIndex(addr))) {
        TRACE_WARNING("Reserving already reserved page at address: 0x%llX", addr.Value());
    }
    ASSERT_EQ(BitMapFree, bitmap_view_.Get(PageIndex(addr)), "Reserving page that is not free");
    bitmap_view_.Set(PageIndex(addr), BitMapAllocated);
}

void PhysicalMemoryManager::Free(PhysicalPtr<void> addr, u64 size)
{
    const u64 start_addr = AlignDown(addr.Value(), kPageSize);
    const u64 end_addr   = AlignUp(addr.Value() + size, kPageSize);
    TRACE_DEBUG("Freeing memory: start=0x%llX, end=0x%llX", start_addr, end_addr);
    for (u64 i = 0; i < end_addr - start_addr; i += kPageSize) {
        Free(PhysicalPtr<void>{start_addr + i});
    }
}

void PhysicalMemoryManager::Free(PhysicalPtr<void> addr)
{
    ASSERT_TRUE(IsAligned(addr.Value(), kPageSize));
    ASSERT_EQ(
        BitMapAllocated, bitmap_view_.Get(PageIndex(addr)), "Freeing page that is not allocated"
    );
    bitmap_view_.Set(PageIndex(addr), BitMapFree);
}

std::expected<PhysicalPtr<void>, MemError> PhysicalMemoryManager::Alloc()
{
    auto res = IterateToNextFreePage();
    if (!res) {
        return std::unexpected{res.error()};
    }

    ASSERT_TRUE(!bitmap_view_.Get(iteration_state_.index), "Allocating already allocated page");
    bitmap_view_.Set(iteration_state_.index, BitMapAllocated);
    return PhysicalPtr<void>{iteration_state_.index * kPageSize};
}

std::expected<PhysicalPtr<void>, MemError> PhysicalMemoryManager::Alloc32()
{
    auto res = IterateToNextFreePage32();
    if (!res) {
        return std::unexpected{res.error()};
    }

    ASSERT_TRUE(!bitmap_view_.Get(iteration_state_.index_32), "Allocating already allocated page");
    bitmap_view_.Set(iteration_state_.index_32, BitMapAllocated);

    ASSERT_LT(iteration_state_.index_32 * kPageSize, kBitMask32);
    return PhysicalPtr<void>{iteration_state_.index_32 * kPageSize};
}

std::expected<PhysicalPtr<void>, MemError> PhysicalMemoryManager::AllocContiguous(u64 size)
{
    if (size == 0) {
        return std::unexpected{MemError::InvalidArgument};
    }

    const u64 num_pages = (size + kPageSize - 1) / kPageSize;

    const u64 total_pages = bitmap_view_.Size();
    if (num_pages > total_pages) {
        return std::unexpected{MemError::OutOfMemory};
    }

    u64 start_pos_hint = iteration_state_.index == 0 ? total_pages : iteration_state_.index;

    for (u64 i = 0; i < total_pages; ++i) {
        const u64 block_start_index = (start_pos_hint + total_pages - 1 - i) % total_pages;

        // A physically contiguous block cannot wrap around the end of physical memory.
        if (block_start_index + num_pages > total_pages) {
            u64 jump = block_start_index - (total_pages - num_pages);
            i += jump;
            continue;
        }

        bool block_is_free = true;
        for (u64 j = 0; j < num_pages; ++j) {
            if (bitmap_view_.Get(block_start_index + j)) {
                block_is_free = false;
                i += j;
                break;
            }
        }

        if (block_is_free) {
            for (u64 j = 0; j < num_pages; ++j) {
                bitmap_view_.Set(block_start_index + j, BitMapAllocated);
            }

            iteration_state_.index = block_start_index;

            return PhysicalPtr<void>{block_start_index * kPageSize};
        }
    }

    return std::unexpected{MemError::OutOfMemory};
}

std::expected<PhysicalPtr<void>, MemError> PhysicalMemoryManager::AllocContiguous32(u64 size)
{
    if (size == 0) {
        return std::unexpected{MemError::InvalidArgument};
    }

    const u64 num_pages = (size + kPageSize - 1) / kPageSize;

    const u64 total_pages = bitmap_view_.Size();
    // The exclusive upper bound for page indices in the 32-bit address space.
    const u64 max_page_index_32_bit = (1ULL << 32) / kPageSize;
    const u64 search_limit          = std::min(total_pages, max_page_index_32_bit);

    if (num_pages > search_limit) {
        return std::unexpected{MemError::OutOfMemory};
    }

    u64 start_pos_hint = iteration_state_.index_32;
    if (start_pos_hint >= search_limit || start_pos_hint == 0) {
        start_pos_hint = search_limit;
    }

    for (u64 i = 0; i < search_limit; ++i) {
        const u64 block_start_index = (start_pos_hint + search_limit - 1 - i) % search_limit;

        // The block must be physically contiguous and stay within the 32-bit limit.
        if (block_start_index + num_pages > search_limit) {
            u64 jump = block_start_index - (search_limit - num_pages);
            i += jump;
            continue;
        }

        bool block_is_free = true;
        for (u64 j = 0; j < num_pages; ++j) {
            if (bitmap_view_.Get(block_start_index + j)) {
                block_is_free = false;
                i += j;
                break;
            }
        }

        if (block_is_free) {
            for (u64 j = 0; j < num_pages; ++j) {
                bitmap_view_.Set(block_start_index + j, BitMapAllocated);
            }

            iteration_state_.index_32 = block_start_index;

            ASSERT_LT(block_start_index * kPageSize, (1ULL << 32));
            return PhysicalPtr<void>{block_start_index * kPageSize};
        }
    }

    return std::unexpected{MemError::OutOfMemory};
}

std::expected<void, MemError> PhysicalMemoryManager::IterateToNextFreePage()
{
    const u64 total_pages = bitmap_view_.Size();
    u64 original_index    = iteration_state_.index;

    do {
        if (iteration_state_.index == 0) {
            iteration_state_.index = total_pages - 1;
        } else {
            --iteration_state_.index;
        }

        if (!bitmap_view_.Get(iteration_state_.index)) {
            return {};  // Found a free page
        }
    } while (iteration_state_.index != original_index);

    return std::unexpected{MemError::OutOfMemory};
}

std::expected<void, MemError> PhysicalMemoryManager::IterateToNextFreePage32()
{
    const u64 total_pages           = bitmap_view_.Size();
    const u64 max_32_bit_page_index = PageIndex(PhysicalPtr<void>{kBitMask32});

    u64 original_index = iteration_state_.index_32;
    do {
        if (iteration_state_.index_32 == 0) {
            iteration_state_.index_32 = std::min(total_pages, max_32_bit_page_index) - 1;
        } else {
            --iteration_state_.index_32;
        }

        if (!bitmap_view_.Get(iteration_state_.index_32)) {
            return {};  // Found a free page
        }
    } while (iteration_state_.index_32 != original_index);

    return std::unexpected{MemError::OutOfMemory};
}

std::tuple<u64, u64> PhysicalMemoryManager::CalcBitmapSize(Multiboot::MemoryMap& mem_map)
{
    using namespace Multiboot;

    u64 maximum_physical_address = 0;
    for (MmapEntry& entry : mem_map) {
        if (entry.type != MmapEntry::kMemoryAvailable) {
            continue;
        }
        maximum_physical_address = std::max(maximum_physical_address, entry.addr + entry.len);
    }

    // Round up division TODO Add to libc
    const u64 total_pages = static_cast<u64>(maximum_physical_address + kPageSize - 1) / kPageSize;
    const u64 bitmap_size = (static_cast<u64>(total_pages) + 7) / 8;

    TRACE_DEBUG(
        "Max avaliable physical address: %#018llx, total pages: %llu, bitmap size: %sB",
        maximum_physical_address, total_pages, FormatMetricUint(bitmap_size)
    );
    return {bitmap_size, total_pages};
}

std::expected<u64, MemError> PhysicalMemoryManager::FindBitmapLocation(
    Multiboot::MemoryMap& mem_map, u64 bitmap_size, u64 lowest_safe_addr
)
{
    using namespace Multiboot;

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
        if (potential_bitmap_start + bitmap_size > kBitMask32) {
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

BitMapView PhysicalMemoryManager::InitBitmapView(const u64 addr, const u64 total_pages)
{
    const u64 size = (total_pages + 7) / 8;
    TRACE_DEBUG("Bitmap located at: %#018llx, size: %sB", addr, FormatMetricUint(size));
    const u64 num_bits     = total_pages;
    BitMapView bitmap_view = BitMapView(reinterpret_cast<void*>(addr), num_bits);
    bitmap_view.SetAll(BitMapAllocated);

    TRACE_DEBUG("Bitmap initialized, verifying...");
    for (u64 i = 0; i < bitmap_view.Size(); ++i) {
        ASSERT_TRUE(bitmap_view.Get(i), "Bitmap not initialized correctly");
    }

    return bitmap_view;
}

void PhysicalMemoryManager::InitFreeMemory(
    PhysicalMemoryManager& pmm, Multiboot::MemoryMap& mem_map
)
{
    using namespace Multiboot;

    for (MmapEntry& entry : mem_map) {
        if (entry.type == MmapEntry::kMemoryAvailable) {
            u64 start_addr = AlignUp<u64>(entry.addr, kPageSize);
            u64 end_addr   = AlignDown<u64>(entry.addr + entry.len, kPageSize);

            if (end_addr <= start_addr) {
                continue;
            }
            pmm.Free(PhysicalPtr<void>{start_addr}, end_addr - start_addr);

            for (u64 addr = start_addr; addr < end_addr; addr += kPageSize) {
                ASSERT_EQ(
                    BitMapFree, pmm.bitmap_view_.Get(PageIndex(PhysicalPtr<void>{addr})),
                    "Page should be free after InitFreeMemory"
                );
            }
        }
    }
}

void PhysicalMemoryManager::InitIterIndicies(
    PhysicalMemoryManager& pmm, Multiboot::MemoryMap& mem_map
)
{
    using namespace Multiboot;

    u64 max_free_addr        = 0;
    u64 max_32_bit_free_addr = 0;

    for (MmapEntry& entry : mem_map) {
        if (entry.type != MmapEntry::kMemoryAvailable) {
            continue;
        }

        const u64 al_entry_start = AlignUp(entry.addr, kPageSize);
        const u64 al_entry_end   = AlignDown(entry.addr + entry.len, kPageSize);

        // 64 bit
        max_free_addr = std::max(max_free_addr, al_entry_end);

        // 32 bit
        if (al_entry_start <= kBitMask32) {
            max_32_bit_free_addr = std::max(max_32_bit_free_addr, al_entry_end);
            max_32_bit_free_addr = std::min(max_32_bit_free_addr, kBitMask32);
        }
    }

    pmm.GetIterationState().index    = PageIndex(PhysicalPtr<void>{max_free_addr});
    pmm.GetIterationState().index_32 = PageIndex(PhysicalPtr<void>{max_32_bit_free_addr});
}
