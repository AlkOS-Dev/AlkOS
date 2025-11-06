#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_

#include <defines.h>
#include <extensions/types.hpp>

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

enum class PageMetaType : u8 {
    Buddy,
    Allocated,
    Dummy,
};

using enum PageMetaType;

struct BuddyMeta {
    VPtr<PageMeta> next;
    VPtr<PageMeta> prev;
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
        AllocatedMeta allocated;
        DummyMeta dummy;
    } data;

    PageMetaType type;
    u8 order;

    // Padding to make the struct size 24 bytes, ensuring 8-byte alignment for
    // the data union.
    u8 _padding[6];

    void InitBuddy(u8 order)
    {
        type            = PageMetaType::Buddy;
        this->order     = order;
        data.buddy.next = nullptr;
        data.buddy.prev = nullptr;
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

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_
