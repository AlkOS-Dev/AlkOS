#include "mem/phys/mngr/slab.hpp"

#include <mutex.hpp>

#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/slab_efficiency.hpp"
#include "modules/memory.hpp"

namespace Mem
{

//==============================================================================
// KmemCache Implementation
//==============================================================================

void KmemCache::Init(
    size_t size, u8 order, size_t num_objs, MetadataSize meta_size, bool off_slab,
    KmemCache *m_cache, BuddyPmm *buddy
)
{
    ASSERT_NOT_ZERO(size);
    ASSERT_NOT_ZERO(num_objs);
    ASSERT_NOT_NULL(buddy, "BuddyPmm must not be null");

    obj_size_      = size;
    block_order_   = order;
    num_objects_   = num_objs;
    metadata_size_ = meta_size;
    is_off_slab_   = off_slab;
    meta_cache_    = m_cache;
    buddy_pmm_     = buddy;

    slabs_partial_ = nullptr;
    slabs_full_    = nullptr;
    slabs_free_    = nullptr;

    num_slabs_total_   = 0;
    num_slabs_partial_ = 0;
    num_slabs_free_    = 0;

    if (is_off_slab_) {
        ASSERT_NOT_NULL(meta_cache_, "Off-slab cache requires a meta cache");
    }
}

size_t KmemCache::GetNextFreeIdx(VPtr<void> base, size_t current_idx) const
{
    VPtr<u8> byte_base = reinterpret_cast<VPtr<u8>>(base);

    switch (metadata_size_) {
        case MetadataSize::Byte: {
            u8 val = byte_base[current_idx];
            if (val == static_cast<u8>(-1))
                return kFreelistSentinel;
            return static_cast<size_t>(val);
        }
        case MetadataSize::Word: {
            u16 val = reinterpret_cast<VPtr<u16>>(byte_base)[current_idx];
            if (val == static_cast<u16>(-1))
                return kFreelistSentinel;
            return static_cast<size_t>(val);
        }
        case MetadataSize::DWord: {
            u32 val = reinterpret_cast<VPtr<u32>>(byte_base)[current_idx];
            if (val == static_cast<u32>(-1))
                return kFreelistSentinel;
            return static_cast<size_t>(val);
        }
        case MetadataSize::QWord: {
            u64 val = reinterpret_cast<VPtr<u64>>(byte_base)[current_idx];
            if (val == static_cast<u64>(-1))
                return kFreelistSentinel;
            return static_cast<size_t>(val);
        }
        default:
            FAIL_ALWAYS("Invalid metadata size");
            return 0;
    }
}

void KmemCache::SetNextFreeIdx(VPtr<void> base, size_t current_idx, size_t next_idx)
{
    VPtr<u8> byte_base = reinterpret_cast<VPtr<u8>>(base);

    switch (metadata_size_) {
        case MetadataSize::Byte:
            byte_base[current_idx] = static_cast<u8>(next_idx);
            break;
        case MetadataSize::Word:
            reinterpret_cast<VPtr<u16>>(byte_base)[current_idx] = static_cast<u16>(next_idx);
            break;
        case MetadataSize::DWord:
            reinterpret_cast<VPtr<u32>>(byte_base)[current_idx] = static_cast<u32>(next_idx);
            break;
        case MetadataSize::QWord:
            reinterpret_cast<VPtr<u64>>(byte_base)[current_idx] = static_cast<u64>(next_idx);
            break;
        default:
            FAIL_ALWAYS("Invalid metadata size");
    }
}

Expected<VPtr<void>, MemError> KmemCache::AllocSlab()
{
    ASSERT_NOT_NULL(buddy_pmm_, "BuddyPmm must be initialized");

    auto res = buddy_pmm_->Alloc({.order = block_order_});
    if (!res) {
        return Unexpected(res.error());
    }

    PPtr<Page> phys_page = *res;
    VPtr<Page> virt_page = PhysToVirt(phys_page);
    VPtr<void> slab_base = reinterpret_cast<VPtr<void>>(virt_page);

    VPtr<void> meta_base = nullptr;

    if (is_off_slab_) {
        ASSERT_NOT_NULL(meta_cache_, "Off-slab cache must be initialized");

        meta_base = meta_cache_->Alloc();
        if (!meta_base) {
            buddy_pmm_->Free(phys_page);
            return Unexpected(MemError::OutOfMemory);
        }
    } else {
        VPtr<u8> bytes = reinterpret_cast<VPtr<u8>>(slab_base);
        meta_base      = reinterpret_cast<VPtr<void>>(bytes + (num_objects_ * obj_size_));
    }

    if (is_off_slab_) {
        // Index 0 of metadata is head index. Initial head is 0.
        SetNextFreeIdx(meta_base, 0, 0);

        for (size_t i = 0; i < num_objects_; ++i) {
            size_t next = (i == num_objects_ - 1) ? kFreelistSentinel : (i + 1);
            SetNextFreeIdx(meta_base, i + 1, next);
        }
    } else {
        for (size_t i = 0; i < num_objects_; ++i) {
            size_t next = (i == num_objects_ - 1) ? kFreelistSentinel : (i + 1);
            SetNextFreeIdx(meta_base, i, next);
        }
    }

    size_t start_pfn = PageFrameNumber(phys_page);
    size_t num_pages = 1 << block_order_;
    auto &pmt        = MemoryModule::Get().GetPageMetaTable();

    for (size_t i = 0; i < num_pages; ++i) {
        PageMeta &meta          = pmt.GetPageMeta(start_pfn + i);
        VPtr<void> freelist_val = nullptr;
        if (is_off_slab_) {
            freelist_val = meta_base;
        } else {
            freelist_val = reinterpret_cast<VPtr<void>>(0);
        }

        meta.InitSlab(block_order_, this, freelist_val, 0);
    }

    PageMeta *slab_meta = &pmt.GetPageMeta(start_pfn);

    AddToPartial(slab_meta);
    num_slabs_total_++;
    num_slabs_partial_++;

    return slab_base;
}

void KmemCache::FreeSlab(PageMeta *slab)
{
    if (is_off_slab_ && meta_cache_) {
        meta_cache_->Free(slab->data.slab.freelist);
    }

    auto &pmt  = MemoryModule::Get().GetPageMetaTable();
    size_t pfn = pmt.GetPageFrameNumber(slab);

    PPtr<Page> page = PageFrameAddr(pfn);
    buddy_pmm_->Free(page);

    num_slabs_total_--;
}

bool KmemCache::Grow()
{
    auto res = AllocSlab();
    return res.has_value();
}

VPtr<void> KmemCache::Alloc()
{
    std::lock_guard guard{lock_};

    if (!slabs_partial_) {
        if (slabs_free_) {
            PageMeta *slab = slabs_free_;
            MoveToPartial(slab);
            num_slabs_free_--;
            num_slabs_partial_++;
        } else {
            if (!Grow()) {
                return nullptr;
            }
        }
    }

    PageMeta *slab = slabs_partial_;
    ASSERT_NOT_NULL(slab, "Must have a partial slab after Grow/Move");

    size_t current_idx   = kFreelistSentinel;
    VPtr<void> meta_base = nullptr;

    auto &pmt            = MemoryModule::Get().GetPageMetaTable();
    size_t pfn           = pmt.GetPageFrameNumber(slab);
    VPtr<void> slab_base = reinterpret_cast<VPtr<void>>(PhysToVirt(PageFrameAddr(pfn)));

    if (is_off_slab_) {
        meta_base   = slab->data.slab.freelist;
        current_idx = GetNextFreeIdx(meta_base, 0);
    } else {
        // For on-slab, freelist in PageMeta stores the head index directly
        current_idx = reinterpret_cast<size_t>(slab->data.slab.freelist);

        VPtr<u8> bytes = reinterpret_cast<VPtr<u8>>(slab_base);
        meta_base      = reinterpret_cast<VPtr<void>>(bytes + (num_objects_ * obj_size_));
    }

    ASSERT_NEQ(current_idx, kFreelistSentinel, "Partial slab must have free objects");

    VPtr<void> obj_addr = reinterpret_cast<VPtr<void>>(
        reinterpret_cast<VPtr<u8>>(slab_base) + (current_idx * obj_size_)
    );

    size_t next_idx = 0;
    if (is_off_slab_) {
        next_idx = GetNextFreeIdx(meta_base, current_idx + 1);
        SetNextFreeIdx(meta_base, 0, next_idx);
    } else {
        next_idx                 = GetNextFreeIdx(meta_base, current_idx);
        slab->data.slab.freelist = reinterpret_cast<VPtr<void>>(next_idx);
    }

    slab->data.slab.inuse++;
    if (slab->data.slab.inuse == num_objects_) {
        MoveToFull(slab);
        num_slabs_partial_--;
    }

    return obj_addr;
}

void KmemCache::Free(VPtr<void> ptr)
{
    PPtr<void> pptr = VirtToPhys(ptr);
    size_t pfn      = PageFrameNumber(pptr);

    auto &pmt      = MemoryModule::Get().GetPageMetaTable();
    PageMeta &meta = pmt.GetPageMeta(pfn);

    size_t head_pfn = pfn & ~((1ULL << meta.order) - 1);
    PageMeta &slab  = pmt.GetPageMeta(head_pfn);

    std::lock_guard guard{lock_};

    R_ASSERT_EQ(slab.type, PageMetaType::Slab, "Page must be a slab");
    R_ASSERT_EQ(slab.data.slab.cache, this, "Pointer does not belong to this cache");

    VPtr<void> slab_base = reinterpret_cast<VPtr<void>>(PhysToVirt(PageFrameAddr(head_pfn)));
    size_t offset        = reinterpret_cast<uptr>(ptr) - reinterpret_cast<uptr>(slab_base);
    size_t obj_idx       = offset / obj_size_;

    VPtr<void> meta_base = nullptr;
    if (is_off_slab_) {
        meta_base       = slab.data.slab.freelist;
        size_t old_head = GetNextFreeIdx(meta_base, 0);

        R_ASSERT_NEQ(obj_idx, old_head, "Double free detected");

        SetNextFreeIdx(meta_base, obj_idx + 1, old_head);
        SetNextFreeIdx(meta_base, 0, obj_idx);
    } else {
        VPtr<u8> bytes = reinterpret_cast<VPtr<u8>>(slab_base);
        meta_base      = reinterpret_cast<VPtr<void>>(bytes + (num_objects_ * obj_size_));

        size_t old_head = reinterpret_cast<size_t>(slab.data.slab.freelist);

        R_ASSERT_NEQ(obj_idx, old_head, "Double free detected");

        SetNextFreeIdx(meta_base, obj_idx, old_head);
        slab.data.slab.freelist = reinterpret_cast<VPtr<void>>(obj_idx);
    }

    slab.data.slab.inuse--;

    if (slab.data.slab.inuse == 0) {
        MoveToFree(&slab);
        // It was either Partial (if it had > 1 inuse before) or Full (if it had 1 inuse and
        // num_objs=1) But typically it comes from Partial.
        if (num_objects_ > 1) {
            num_slabs_partial_--;
        }
        num_slabs_free_++;
    } else if (slab.data.slab.inuse == num_objects_ - 1) {
        MoveToPartial(&slab);
        num_slabs_partial_++;
    }
}

void KmemCache::AddToPartial(PageMeta *slab)
{
    slab->data.slab.next = slabs_partial_;
    slab->data.slab.prev = nullptr;
    if (slabs_partial_)
        slabs_partial_->data.slab.prev = slab;
    slabs_partial_ = slab;
}

void KmemCache::AddToFull(PageMeta *slab)
{
    slab->data.slab.next = slabs_full_;
    slab->data.slab.prev = nullptr;
    if (slabs_full_)
        slabs_full_->data.slab.prev = slab;
    slabs_full_ = slab;
}

void KmemCache::AddToFree(PageMeta *slab)
{
    slab->data.slab.next = slabs_free_;
    slab->data.slab.prev = nullptr;
    if (slabs_free_)
        slabs_free_->data.slab.prev = slab;
    slabs_free_ = slab;
}

void KmemCache::RemoveFromList(PageMeta *slab)
{
    if (slab->data.slab.prev) {
        slab->data.slab.prev->data.slab.next = slab->data.slab.next;
    } else {
        if (slabs_partial_ == slab)
            slabs_partial_ = slab->data.slab.next;
        else if (slabs_full_ == slab)
            slabs_full_ = slab->data.slab.next;
        else if (slabs_free_ == slab)
            slabs_free_ = slab->data.slab.next;
    }

    if (slab->data.slab.next) {
        slab->data.slab.next->data.slab.prev = slab->data.slab.prev;
    }

    slab->data.slab.next = nullptr;
    slab->data.slab.prev = nullptr;
}

void KmemCache::MoveToPartial(PageMeta *slab)
{
    RemoveFromList(slab);
    AddToPartial(slab);
}
void KmemCache::MoveToFull(PageMeta *slab)
{
    RemoveFromList(slab);
    AddToFull(slab);
}
void KmemCache::MoveToFree(PageMeta *slab)
{
    RemoveFromList(slab);
    AddToFree(slab);
}

//==============================================================================
// SlabAllocator Implementation
//==============================================================================

template <size_t Index>
void SlabAllocator::InitCacheHelper(KmemCache &cache, BuddyPmm &buddy)
{
    constexpr size_t ObjSize = 1ULL << (Index + SlabEfficiency::kMinObjectSizeLog2);
    constexpr u8 Order       = SlabEfficiency::kSlabEfficiencyTable[Index][0];

    constexpr size_t kSmallObjectLimit = 512;
    constexpr bool ForceOffSlab        = (ObjSize >= kSmallObjectLimit);
    bool off_slab                      = ForceOffSlab;

    size_t block_size = BuddyPmm::BuddyAreaSize(Order);
    size_t num_objs   = 0;

    using MetaSize     = KmemCache::MetadataSize;
    MetaSize meta_size = MetaSize::Byte;

    KmemCache *meta_cache = nullptr;

    if (off_slab) {
        num_objs = block_size / ObjSize;

        if (num_objs < 256)
            meta_size = MetaSize::Byte;
        else if (num_objs < 65536)
            meta_size = MetaSize::Word;
        else
            meta_size = MetaSize::DWord;

        size_t meta_bytes = static_cast<size_t>(meta_size);
        size_t array_size = (num_objs + 1) * meta_bytes;
        meta_cache        = GetCache(array_size);
    } else {
        using Info           = SlabEfficiency::SlabEfficiencyInfo<ObjSize, Order>;
        num_objs             = Info::kCapacity;
        size_t raw_meta_size = Info::kSizeOfMetadataPerObj;

        if (raw_meta_size == 1)
            meta_size = MetaSize::Byte;
        else if (raw_meta_size == 2)
            meta_size = MetaSize::Word;
        else if (raw_meta_size == 4)
            meta_size = MetaSize::DWord;
        else
            meta_size = MetaSize::QWord;
    }

    cache.Init(ObjSize, Order, num_objs, meta_size, off_slab, meta_cache, &buddy);
}

template <size_t... Is>
void SlabAllocator::InitCaches(std::index_sequence<Is...>, BuddyPmm &buddy)
{
    ((InitCacheHelper<Is>(caches_[Is], buddy)), ...);
}

void SlabAllocator::Init(BuddyPmm &buddy)
{
    InitCaches(std::make_index_sequence<kNumSizeClasses>{}, buddy);
}

KmemCache *SlabAllocator::GetCache(size_t size)
{
    if (size == 0)
        return nullptr;

    size_t idx = 0;
    size_t s   = 8;
    while (s < size && idx < kNumSizeClasses) {
        s <<= 1;
        idx++;
    }

    if (idx >= kNumSizeClasses)
        return nullptr;

    return &caches_[idx];
}

KmemCache *SlabAllocator::GetCacheFromIndex(size_t index)
{
    if (index >= kNumSizeClasses)
        return nullptr;
    return &caches_[index];
}

}  // namespace Mem
