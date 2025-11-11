#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include <assert.h>
#include <limits>
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
    // This slab implementation allocates a block of memory from the buddy
    // allocator and partitions it into two main sections:
    //
    // 1. Freelist Metadata: An array of `FreeListItem` (u16) indices that
    //    forms a linked list of available object slots. This metadata is
    //    stored at the beginning of the block.
    //
    // 2. Object Storage: The remaining space in the block is used to store
    //    the actual objects of type T. This area is located at the end of
    //    the block.
    //
    // Memory Layout:
    // [ Freelist Metadata | Object Storage | Padding (if any) ]
    //   ^-- block_start

    public:
    using StoredType                               = T;
    static constexpr size_t kStoredObjSize         = sizeof(StoredType);
    static constexpr f64 kTolerableSpaceEfficiency = 0.95F;

    static constexpr u8 kBlockOrder = FindBestBlockOrder();

    using BestFitInfo  = BestFit<kBlockOrder>;
    using FreeListItem = typename BestFitInfo::FreeListItemType;

    static constexpr size_t kSizeOfMetadataPerObj   = BestFitInfo::kSizeOfMetadataPerObj;
    static constexpr size_t kTotalStoredBytesPerObj = BestFitInfo::kTotalStoredBytesPerObj;
    static constexpr FreeListItem kFreelistSentiel  = -1;

    static_assert(
        BestFitInfo::kCapacity < kFreelistSentiel,
        "Number of objects in slab exceeds FreeListItem capacity"
    );

    Slab(BuddyPmm &b_pmm)
    {
        Expected<PPtr<Page>, MemError> res = b_pmm.Alloc({.order = kBlockOrder});
        R_ASSERT_TRUE(res, "Not enough mem for slab");
        PPtr<Page> p1 = *res;
        VPtr<Page> p2 = PhysToVirt(p1);

        VPtr<u8> block_start     = reinterpret_cast<VPtr<u8>>(p2);
        const size_t num_objects = BestFitInfo::kCapacity;

        VPtr<FreeListItem> freelist_start = reinterpret_cast<VPtr<FreeListItem>>(block_start);
        freelist_table                    = std::span(freelist_start, num_objects);

        VPtr<u8> obj_start_as_byte_ptr = block_start + (num_objects * kSizeOfMetadataPerObj);
        VPtr<T> obj_start              = reinterpret_cast<VPtr<T>>(obj_start_as_byte_ptr);
        object_table                   = std::span(obj_start, num_objects);

        for (size_t i = 0; i < num_objects; i++) {
            freelist_table[i] = i + 1;
        }
        freelist_table.back() = kFreelistSentiel;
        freelist_head         = 0;
    };

    Expected<VPtr<StoredType>, MemError> Alloc()
    {
        if (freelist_head == kFreelistSentiel) {
            return MemError::kOutOfMemory;
        }

        FreeListItem current_idx = freelist_head;
        freelist_head            = freelist_table[current_idx];

        return &object_table[current_idx];
    }

    void Free(VPtr<StoredType> obj_ptr)
    {
        ptrdiff_t idx = obj_ptr - object_table.data();

        ASSERT_GE(idx, 0, "Freeing invalid object");
        ASSERT_LT(idx, num_objects, "Freeing invalid object");

        freelist_table[idx] = freelist_head;
        freelist_head       = idx;
    }

    private:
    template <u8 Order>
    static constexpr u8 FindBestOrderHelper()
    {
        if constexpr (Order >= BuddyPmm::kMaxOrder) {
            return BuddyPmm::kMaxOrder - 1;
        } else if constexpr (BestFit<Order>::kSpaceEfficiency >= kTolerableSpaceEfficiency) {
            return Order;
        } else {
            return FindBestOrderHelper<Order + 1>();
        }
    }

    static constexpr u8 FindBestBlockOrder() { return FindBestOrderHelper<0>(); };

    template <u8 BlockOrder>
    struct BestFit {
        static constexpr size_t kBlockSize = BuddyPmm::BuddyAreaSize(BlockOrder);

        using FreeListItemType = decltype([] {
            constexpr size_t capacity_u8 = kBlockSize / (kStoredObjSize + sizeof(u8));
            if constexpr (capacity_u8 < std::numeric_limits<u8>::max()) {
                return u8{};
            }
            constexpr size_t capacity_u16 = kBlockSize / (kStoredObjSize + sizeof(u16));
            if constexpr (capacity_u16 < std::numeric_limits<u16>::max()) {
                return u16{};
            }
            constexpr size_t capacity_u32 = kBlockSize / (kStoredObjSize + sizeof(u32));
            if constexpr (capacity_u32 < std::numeric_limits<u32>::max()) {
                return u32{};
            }
            return u64{};
        }());

        static constexpr size_t kSizeOfMetadataPerObj   = sizeof(FreeListItemType);
        static constexpr size_t kTotalStoredBytesPerObj = kStoredObjSize + kSizeOfMetadataPerObj;

        static constexpr size_t kCapacity = kBlockSize / kTotalStoredBytesPerObj;
        static constexpr f64 kSpaceEfficiency =
            static_cast<f64>(kCapacity * kTotalStoredBytesPerObj) / kBlockSize;
    };

    std::span<T> object_table{};
    std::span<FreeListItem> freelist_table{};
    FreeListItem freelist_head = kFreelistSentiel;
};

template <class T>
class Slab<T, kOffSlabFreelist>
{
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
