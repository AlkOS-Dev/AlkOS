#ifndef ALKOS_BOOT_LIB_MEM_PMM_HPP_
#define ALKOS_BOOT_LIB_MEM_PMM_HPP_

#include <extensions/algorithm.hpp>
#include <extensions/bit.hpp>
#include <extensions/bit_array.hpp>
#include <extensions/debug.hpp>
#include <extensions/expected.hpp>
#include <extensions/internal/formats.hpp>
#include <extensions/optional.hpp>
#include <extensions/utility.hpp>

#include "mem/error.hpp"
#include "mem/page_map.hpp"
#include "mem/physical_ptr.hpp"

#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

class PhysicalMemoryManager
{
    // TODO
    // Should add overflow checked arithmetic to libc and use it here

    //==============================================================================
    // Structs and Enums
    // ==============================================================================

    private:
    static constexpr u64 kPageSize = PageSize<PageSizeTag::k4Kb>();

    enum {
        BitMapFree      = 0,
        BitMapAllocated = 1,
    };

    struct IterationState {
        size_t index;
        size_t index_32;
    } PACK;

    public:
    struct PmmState {
        u64 bitmap_addr;
        u64 total_pages;
        size_t iteration_index;
        size_t iteration_index_32;
    } PACK;

    //==============================================================================
    // Class Creation
    //==============================================================================

    explicit PhysicalMemoryManager(const PmmState& state)
        : bitmap_view_{reinterpret_cast<void*>(state.bitmap_addr), state.total_pages},
          iteration_state_{state.iteration_index, state.iteration_index_32}
    {
    }

    static std::expected<PhysicalMemoryManager, MemError> Create(
        Multiboot::MemoryMap mem_map, u64 lowest_safe_addr
    );

    public:
    //==============================================================================
    // Public Methods
    //==============================================================================

    void Reserve(PhysicalPtr<void> addr, u64 size);
    void Reserve(PhysicalPtr<void> addr);

    void Free(PhysicalPtr<void> addr, u64 size);
    void Free(PhysicalPtr<void> addr);

    std::expected<PhysicalPtr<void>, MemError> Alloc();
    std::expected<PhysicalPtr<void>, MemError> Alloc32();
    std::expected<PhysicalPtr<void>, MemError> AllocContiguous(u64 size);
    std::expected<PhysicalPtr<void>, MemError> AllocContiguous32(u64 size);

    FORCE_INLINE_F PmmState GetState()
    {
        return PmmState{
            .bitmap_addr        = GetBitmapAddress().Value(),
            .total_pages        = GetTotalPages(),
            .iteration_index    = GetIterationState().index,
            .iteration_index_32 = GetIterationState().index_32
        };
    }

    FORCE_INLINE_F PhysicalPtr<void> GetBitmapAddress()
    {
        return PhysicalPtr<void>(bitmap_view_.Storage());
    }
    FORCE_INLINE_F u64 GetTotalPages() const { return bitmap_view_.Size(); }
    FORCE_INLINE_F IterationState& GetIterationState() { return iteration_state_; }
    FORCE_INLINE_F IterationState GetIterationState() const { return iteration_state_; }
    FORCE_INLINE_F void SetIterationState(IterationState state) { iteration_state_ = state; }

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    static FORCE_INLINE_F u64 PageIndex(PhysicalPtr<void> addr) { return addr.Value() / kPageSize; }

    std::expected<void, MemError> IterateToNextFreePage();
    std::expected<void, MemError> IterateToNextFreePage32();

    // Init Helpers

    static IterationState InitializeIterationIndices(Multiboot::MemoryMap& mem_map);

    static std::tuple<u64, u64> CalcBitmapSize(Multiboot::MemoryMap& mem_map);

    static std::expected<u64, MemError> FindBitmapLocation(
        Multiboot::MemoryMap& mem_map, u64 bitmap_size, u64 lowest_safe_addr
    );

    static BitMapView InitBitmapView(const u64 addr, const u64 size);

    static void InitFreeMemory(PhysicalMemoryManager& pmm, Multiboot::MemoryMap& mem_map);

    //==============================================================================
    // Private fields
    //==============================================================================

    BitMapView bitmap_view_;
    IterationState iteration_state_{};
};

#endif  // ALKOS_BOOT_LIB_MEM_PMM_HPP_
