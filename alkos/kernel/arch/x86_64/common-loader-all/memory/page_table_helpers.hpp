#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_

#include <extensions/types.hpp>
#include "defines.hpp"
#include "page_tables_layout.hpp"

namespace memory
{
template <u8 level>
struct PageTableTypeGetter {
    using Type = void;
};

template <>
struct PageTableTypeGetter<1> {
    using Type = PML1_t;
};

template <>
struct PageTableTypeGetter<2> {
    using Type = PML2_t;
};

template <>
struct PageTableTypeGetter<3> {
    using Type = PML3_t;
};

template <>
struct PageTableTypeGetter<4> {
    using Type = PML4_t;
};

template <u8 level>
struct PageEntryTypeGetter {
    using Type = void;
};

template <>
struct PageEntryTypeGetter<1> {
    using Type = PML1Entry_t;
};

template <>
struct PageEntryTypeGetter<2> {
    using Type = PML2Entry_t;
};

template <>
struct PageEntryTypeGetter<3> {
    using Type = PML3Entry_t;
};

template <>
struct PageEntryTypeGetter<4> {
    using Type = PML4Entry_t;
};

template <class PageTableOrEntry>
struct PageLevelGetter {
    static constexpr u8 value = 0;
};

template <>
struct PageLevelGetter<PML1_t> {
    static constexpr u8 value = 1;
};

template <>
struct PageLevelGetter<PML2_t> {
    static constexpr u8 value = 2;
};

template <>
struct PageLevelGetter<PML3_t> {
    static constexpr u8 value = 3;
};

template <>
struct PageLevelGetter<PML4_t> {
    static constexpr u8 value = 4;
};

template <>
struct PageLevelGetter<PML1Entry_t> {
    static constexpr u8 value = 1;
};

template <>
struct PageLevelGetter<PML2Entry_t> {
    static constexpr u8 value = 2;
};

template <>
struct PageLevelGetter<PML3Entry_t> {
    static constexpr u8 value = 3;
};

template <>
struct PageLevelGetter<PML4Entry_t> {
    static constexpr u8 value = 4;
};

}  // namespace memory

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_
