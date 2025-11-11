#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include <span.hpp>

#include "hal/constants.hpp"
#include "mem/phys/mngr/buddy.hpp"

namespace Mem
{

/// TODO: Current implementation doesn't align objects
/// and ignores their desired alignment

static constexpr bool kOnSlabFreelist  = true;
static constexpr bool kOffSlabFreelist = false;

template <class T, bool OnSlabFreeList = true>
class Slab
{
    public:
    // In constructor
    // Calc size of stored object
    // If size(T) > 512 -> OffSlabFreelist = true
    //
    // | First init Cache-Cache
    // | Slab<OffSlabFreelist = false>() sizes from 8 to 512
    // | OnSlabFreeList -> Self Contained Class (Except Buddy dependency)
    // | Allocates normal page and manages it -> Marks it as used by slab
    // | Alloc -> Gives item from freelist
    // | Free  -> Returns item to freelist
    //
    // If size(T) > 512:
    // Use Cache-Cache to init OffSlabFreeList
    // Rest is same
    //
    // Then make Fasade to create Slabs in simple manner
};

template <class T>
class Slab<T, kOnSlabFreelist>
{
    public:
    using StoredType   = T;
    using FreeListItem = u8;

    static constexpr size_t kStoredObjSize          = sizeof(StoredType);
    static constexpr size_t kSizeOfMetadataPerObj   = sizeof(FreeListItem);
    static constexpr size_t kTotalStoredBytesPerObj = kStoredObjSize + kSizeOfMetadataPerObj;
    static constexpr u8 kFreelistSentiel            = -1;

    static constexpr f64 kTolerableSpaceEfficiency = 0.95F;

    static constexpr size_t FindBestBlockOrder()
    {
        u8 found_order = 0;
        for (u8 order = 0; order < BuddyPmm::kMaxOrder; order++) {
            found_order = order;
            if (CalcSpaceEfficiency(order) >= kTolerableSpaceEfficiency) {
                break;
            }
        }
        return found_order;
    };
    static constexpr u8 kBlockOrder = FindBestBlockOrder();

    Slab(BuddyPmm &b_pmm)
    {
        Expected<PPtr<Page>, MemError> res = b_pmm.Alloc({.order = kBlockOrder});
        R_ASSERT_TRUE(res, "Not enough mem for slab");
        PPtr<Page> p1 = *res;
        VPtr<Page> p2 = PhysToVirt(p1);

        VPtr<u8> block_start = reinterpret_cast<VPtr<u8>>(p2);
        num_objects          = CalcCapacity(kBlockOrder);

        VPtr<FreeListItem> freelist_start = reinterpret_cast<VPtr<FreeListItem>>(block_start);
        free_list_table                   = std::span(freelist_start, num_objects);

        size_t bas                     = BuddyPmm::BuddyAreaSize(kBlockOrder);
        VPtr<u8> obj_start_as_byte_ptr = block_start + bas - (num_objects * kStoredObjSize);
        VPtr<T> obj_start              = reinterpret_cast<VPtr<T>>(obj_start_as_byte_ptr);
        object_table                   = std::span(obj_start, num_objects);

        for (size_t i = 0; i < num_objects; i++) {
            free_list_table[i] = i + 1;
        }
        free_list_table.back() = kFreelistSentiel;
    };

    private:
    static constexpr size_t CalcCapacity(u8 block_order)
    {
        size_t buddy_area_size = BuddyPmm::BuddyAreaSize(block_order);
        size_t capacity        = buddy_area_size / (kTotalStoredBytesPerObj);
        return capacity;
    };

    static constexpr f64 CalcSpaceEfficiency(u8 block_order)
    {
        size_t capacity        = CalcCapacity(block_order);
        size_t buddy_area_size = BuddyPmm::BuddyAreaSize(block_order);
        f64 efficiency = static_cast<f64>(buddy_area_size) / (capacity * kTotalStoredBytesPerObj);
        return efficiency;
    };

    std::span<T> object_table{};
    std::span<FreeListItem> free_list_table{};
    size_t num_objects{};
};

template <class T>
class Slab<T, kOffSlabFreelist>
{
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
