#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include <assert.h>
#include <limits.hpp>
#include <span.hpp>

#include "hal/constants.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/phys/mngr/slab_efficiency.hpp"
#include "mem/types.hpp"

namespace Mem
{

static constexpr bool kOnSlabFreelist  = true;
static constexpr bool kOffSlabFreelist = false;

template <class T, u8 BlockOrder, bool OnSlabFreeList = true>
class Slab
{
};

template <class T, u8 BlockOrder>
class Slab<T, BlockOrder, kOnSlabFreelist>
{
    private:
    using EfficiencyInfo = SlabEfficiency::SlabEfficiencyInfo<sizeof(T), BlockOrder>;

    public:
    using StoredType = T;

    using FreeListItem = typename EfficiencyInfo::FreeListItemType;

    static constexpr FreeListItem kFreelistSentiel = static_cast<FreeListItem>(-1);

    static_assert(
        EfficiencyInfo::kCapacity < kFreelistSentiel,
        "Number of objects in slab exceeds FreeListItem capacity"
    );

    Slab(BuddyPmm &b_pmm)
    {
        Expected<PPtr<Page>, MemError> res = b_pmm.Alloc({.order = BlockOrder});
        R_ASSERT_TRUE(res, "Not enough mem for slab");
        PPtr<Page> p1 = *res;
        VPtr<Page> p2 = PhysToVirt(p1);

        VPtr<u8> block_start     = reinterpret_cast<VPtr<u8>>(p2);
        const size_t num_objects = EfficiencyInfo::kCapacity;

        VPtr<FreeListItem> freelist_start = reinterpret_cast<VPtr<FreeListItem>>(block_start);
        freelist_table                    = std::span<FreeListItem>(freelist_start, num_objects);

        VPtr<u8> obj_start_as_byte_ptr =
            block_start + (num_objects * EfficiencyInfo::kSizeOfMetadataPerObj);
        VPtr<T> obj_start = reinterpret_cast<VPtr<T>>(obj_start_as_byte_ptr);
        object_table      = std::span<T>(obj_start, num_objects);

        for (size_t i = 0; i < num_objects; i++) {
            freelist_table[i] = i + 1;
        }
        freelist_table.back() = kFreelistSentiel;
        freelist_head         = 0;
    };

    Expected<VPtr<StoredType>, MemError> Alloc()
    {
        if (freelist_head == kFreelistSentiel) {
            return Unexpected(MemError::OutOfMemory);
        }

        FreeListItem current_idx = freelist_head;
        freelist_head            = freelist_table[current_idx];

        return &object_table[current_idx];
    }

    void Free(VPtr<StoredType> obj_ptr)
    {
        const auto obj_uptr         = PtrToUptr(obj_ptr);
        const auto table_start_uptr = PtrToUptr(object_table.data());

        ASSERT_GE(obj_uptr, table_start_uptr, "Freeing invalid object (out of bounds low)");

        const size_t offset = obj_uptr - table_start_uptr;
        ASSERT_EQ(offset % sizeof(StoredType), 0, "Freeing object with invalid alignment");

        const size_t idx = offset / sizeof(StoredType);
        ASSERT_LT(idx, object_table.size(), "Freeing invalid object (out of bounds high)");

        freelist_table[idx] = freelist_head;
        freelist_head       = idx;
    }

    // TODO: Deconstructor & Destroy method

    private:
    std::span<T> object_table{};
    std::span<FreeListItem> freelist_table{};
    FreeListItem freelist_head = kFreelistSentiel;
};

template <class T, u8 BlockOrder>
class Slab<T, BlockOrder, kOffSlabFreelist>
{
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
