#ifndef KERNEL_SRC_MEM_PAGE_META_HPP_
#define KERNEL_SRC_MEM_PAGE_META_HPP_

#include <defines.h>
#include <types.hpp>

#include "mem/types.hpp"

//==============================================================================
// This file declares structures to store metadata of all pages in memory
//
// Pages are reused with different memory layout depending on their current
// purpose
//==============================================================================

namespace Mem
{
struct PageMeta;
class KmemCache;

enum class PageMetaType : u8 {
    Buddy,
    Allocated,
    Slab,
    Dummy,
};

using enum PageMetaType;

struct BuddyMeta {
    VPtr<PageMeta> next;
    VPtr<PageMeta> prev;
} PACK;

struct SlabMeta {
    VPtr<PageMeta> next;
    VPtr<PageMeta> prev;
    KmemCache *cache;
    VPtr<void> freelist;
    u16 inuse;
} PACK;

struct AllocatedMeta {
    u8 _unused[16];
} PACK;

struct DummyMeta {
    u8 _unused[16];
} PACK;

struct PageMeta {
    union {
        BuddyMeta buddy;
        SlabMeta slab;
        AllocatedMeta allocated;
        DummyMeta dummy;
    } data;

    PageMetaType type;
    u8 order;

    // Padding to make the struct size 40 bytes, ensuring 8-byte alignment for
    // the data union.
    u8 _padding[4];

    void InitBuddy(u8 order)
    {
        type            = PageMetaType::Buddy;
        this->order     = order;
        data.buddy.next = nullptr;
        data.buddy.prev = nullptr;
    }

    void InitSlab(u8 order, KmemCache *cache, VPtr<void> freelist, u16 inuse = 0)
    {
        type               = PageMetaType::Slab;
        this->order        = order;
        data.slab.next     = nullptr;
        data.slab.prev     = nullptr;
        data.slab.cache    = cache;
        data.slab.freelist = freelist;
        data.slab.inuse    = inuse;
    }

    void InitAllocated(u8 order)
    {
        type        = PageMetaType::Allocated;
        this->order = order;
    }

    static BuddyMeta &AsBuddy(PageMeta &meta)
    {
        ASSERT_EQ(meta.type, PageMetaType::Buddy);
        return meta.data.buddy;
    }

    static SlabMeta &AsSlab(PageMeta &meta)
    {
        ASSERT_EQ(meta.type, PageMetaType::Slab);
        return meta.data.slab;
    }

    static AllocatedMeta &AsAllocated(PageMeta &meta)
    {
        ASSERT_EQ(meta.type, PageMetaType::Allocated);
        return meta.data.allocated;
    }

    static DummyMeta &AsDummy(PageMeta &meta)
    {
        ASSERT_EQ(meta.type, PageMetaType::Dummy);
        return meta.data.dummy;
    }
} PACK;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_PAGE_META_HPP_
