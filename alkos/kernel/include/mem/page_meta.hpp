#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_

#include <defines.h>
#include <extensions/types.hpp>

//==============================================================================
// This file declares structures to store metadata of all pages in memory
//
// Pages are reused with different memory layout depending on their current
// purpose
//==============================================================================

namespace Mem
{
enum class PageMetaType : u8 {
    Buddy,
    Virtual,
    Dummy,
};

using enum PageMetaType;

template <PageMetaType T = Dummy>
struct PageMeta {
    PageMetaType type;
} PACK;

template <>
struct PageMeta<Buddy> {
    PageMetaType type;
} PACK;

template <>
struct PageMeta<Virtual> {
    PageMetaType type;
} PACK;

static constexpr size_t kExpectedSize = sizeof(PageMeta<Dummy>);
static_assert(sizeof(PageMeta<Buddy>) == kExpectedSize);
static_assert(sizeof(PageMeta<Virtual>) == kExpectedSize);
}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_HPP_
