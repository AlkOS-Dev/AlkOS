#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include <assert.h>
#include <limits.hpp>
#include <span.hpp>

#include "hal/constants.hpp"
#include "mem/phys/mngr/buddy.hpp"

namespace Mem
{

static constexpr bool kOnSlabFreelist  = true;
static constexpr bool kOffSlabFreelist = false;

template <class T, bool OnSlabFreeList = true>
class Slab
{
};

template <class T>
class Slab<T, kOnSlabFreelist>
{
    private:
    template <u8 BlockOrder>
    struct BestFit {
        static constexpr size_t kBlockSize = BuddyPmm::BuddyAreaSize(BlockOrder);

        using FreeListItemType = decltype([] {
            constexpr size_t capacity_u8 = kBlockSize / (sizeof(T) + sizeof(u8));
            if constexpr (capacity_u8 < std::numeric_limits<u8>::max()) {
                return u8{};
            }
            constexpr size_t capacity_u16 = kBlockSize / (sizeof(T) + sizeof(u16));
            if constexpr (capacity_u16 < std::numeric_limits<u16>::max()) {
                return u16{};
            }
            constexpr size_t capacity_u32 = kBlockSize / (sizeof(T) + sizeof(u32));
            if constexpr (capacity_u32 < std::numeric_limits<u32>::max()) {
                return u32{};
            }
            return u64{};
        }());

        static constexpr size_t kSizeOfMetadataPerObj   = sizeof(FreeListItemType);
        static constexpr size_t kTotalStoredBytesPerObj = sizeof(T) + kSizeOfMetadataPerObj;

        static constexpr size_t kCapacity = kBlockSize / kTotalStoredBytesPerObj;
        static constexpr f64 kSpaceEfficiency =
            static_cast<f64>(kCapacity * kTotalStoredBytesPerObj) / kBlockSize;
    };

    template <u8 Order>
    static constexpr u8 FindBestOrderHelper()
    {
        if constexpr (Order >= BuddyPmm::kMaxOrder) {
            return BuddyPmm::kMaxOrder - 1;
        } else if constexpr (BestFit<Order>::kSpaceEfficiency >= 0.95F) {
            return Order;
        } else {
            return FindBestOrderHelper<Order + 1>();
        }
    }

    static constexpr u8 FindBestBlockOrder() { return FindBestOrderHelper<0>(); }

    public:
    using StoredType                = T;
    static constexpr u8 kBlockOrder = FindBestBlockOrder();

    using BestFitInfo  = BestFit<kBlockOrder>;
    using FreeListItem = typename BestFitInfo::FreeListItemType;

    static constexpr FreeListItem kFreelistSentiel = static_cast<FreeListItem>(-1);

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

        VPtr<u8> obj_start_as_byte_ptr =
            block_start + (num_objects * BestFitInfo::kSizeOfMetadataPerObj);
        VPtr<T> obj_start = reinterpret_cast<VPtr<T>>(obj_start_as_byte_ptr);
        object_table      = std::span(obj_start, num_objects);

        for (size_t i = 0; i < num_objects; i++) {
            freelist_table[i] = i + 1;
        }
        freelist_table.back() = kFreelistSentiel;
        freelist_head         = 0;
    };

    Expected<VPtr<StoredType>, MemError> Alloc()
    {
        if (freelist_head == kFreelistSentiel) {
            return MemError::OutOfMemory;
        }

        FreeListItem current_idx = freelist_head;
        freelist_head            = freelist_table[current_idx];

        return &object_table[current_idx];
    }

    void Free(VPtr<StoredType> obj_ptr)
    {
        ptrdiff_t idx = obj_ptr - object_table.data();

        ASSERT_GE(idx, 0, "Freeing invalid object");
        ASSERT_LT(idx, BestFitInfo::kCapacity, "Freeing invalid object");

        freelist_table[idx] = freelist_head;
        freelist_head       = idx;
    }

    private:
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
