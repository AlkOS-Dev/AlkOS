#ifndef KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
#define KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_

#include <assert.h>
#include <limits.hpp>
#include <span.hpp>

#include "hal/constants.hpp"
#include "mem/page_meta.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/types.hpp"
#include "sync/kernel/spinlock.hpp"

namespace Mem
{

class SlabAllocator;

class KmemCache
{
    public:
    enum class MetadataSize : size_t {
        Byte  = 1,
        Word  = 2,
        DWord = 4,
        QWord = 8,
    };

    static constexpr size_t kFreelistSentinel = static_cast<size_t>(-1);

    KmemCache() = default;

    void Init(
        size_t size, u8 order, size_t num_objs, MetadataSize meta_size, bool off_slab,
        VPtr<KmemCache> meta_cache, VPtr<BuddyPmm> buddy
    );

    VPtr<void> Alloc();
    void Free(VPtr<void> ptr);

    private:
    Spinlock lock_;
    size_t obj_size_{0};
    size_t num_objects_{0};
    MetadataSize metadata_size_{MetadataSize::Byte};
    u8 block_order_{0};
    bool is_off_slab_{false};

    // Pointer to the cache used for off-slab freelist arrays
    VPtr<KmemCache> meta_cache_{nullptr};
    VPtr<BuddyPmm> buddy_pmm_{nullptr};

    // Lists of slabs (PageMeta acting as slab descriptors)
    VPtr<PageMeta> slabs_partial_{nullptr};
    VPtr<PageMeta> slabs_full_{nullptr};
    VPtr<PageMeta> slabs_free_{nullptr};

    // Stats
    size_t num_slabs_total_{0};
    size_t num_slabs_partial_{0};
    size_t num_slabs_free_{0};

    Expected<VPtr<void>, MemError> AllocSlab();
    void FreeSlab(VPtr<PageMeta> slab);
    bool Grow();

    // List management helpers
    void AddToPartial(VPtr<PageMeta> slab);
    void AddToFull(VPtr<PageMeta> slab);
    void AddToFree(VPtr<PageMeta> slab);
    void RemoveFromList(VPtr<PageMeta> slab);
    void MoveToPartial(VPtr<PageMeta> slab);
    void MoveToFull(VPtr<PageMeta> slab);
    void MoveToFree(VPtr<PageMeta> slab);

    // Freelist helpers
    size_t GetNextFreeIdx(VPtr<void> base, size_t current_idx) const;
    void SetNextFreeIdx(VPtr<void> base, size_t current_idx, size_t next_idx);

    friend class SlabAllocator;
};

class SlabAllocator
{
    public:
    static constexpr size_t kNumSizeClasses = 10;  // 8 to 4096
    // Size classes: 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096

    SlabAllocator() = default;

    void Init(BuddyPmm &buddy);

    VPtr<KmemCache> GetCache(size_t size);
    VPtr<KmemCache> GetCacheFromIndex(size_t index);

    private:
    std::array<KmemCache, kNumSizeClasses> caches_;

    template <size_t Index>
    void InitCacheHelper(KmemCache &cache, BuddyPmm &buddy);

    template <size_t... Is>
    void InitCaches(std::index_sequence<Is...>, BuddyPmm &buddy);
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PHYS_MNGR_SLAB_HPP_
