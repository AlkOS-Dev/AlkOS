#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_

#include <extensions/types.hpp>
#include "defines.hpp"
#include "page_tables_layout.hpp"

namespace memory
{

namespace detail
{
namespace page_table_helpers
{
// --- Central Traits Struct ---
template <u8 level>
struct PageLevelTraits {
    using TableType               = void;
    using EntryType               = void;
    static constexpr bool isValid = false;
};

template <>
struct PageLevelTraits<1> {
    using TableType               = PML1_t;
    using EntryType               = PML1Entry_t;
    static constexpr u8 level     = 1;
    static constexpr bool isValid = true;
};

template <>
struct PageLevelTraits<2> {
    using TableType               = PML2_t;
    using EntryType               = PML2Entry_t;
    static constexpr u8 level     = 2;
    static constexpr bool isValid = true;
};

template <>
struct PageLevelTraits<3> {
    using TableType               = PML3_t;
    using EntryType               = PML3Entry_t;
    static constexpr u8 level     = 3;
    static constexpr bool isValid = true;
};

template <>
struct PageLevelTraits<4> {
    using TableType               = PML4_t;
    using EntryType               = PML4Entry_t;
    static constexpr u8 level     = 4;
    static constexpr bool isValid = true;
};
}  // namespace page_table_helpers
}  // namespace detail

// Get Table type from level
template <u8 level>
using PageTableT = typename detail::page_table_helpers::PageLevelTraits<level>::TableType;

// Get Entry type from level
template <u8 level>
using PageEntryT = typename detail::page_table_helpers::PageLevelTraits<level>::EntryType;

// Get Level from Table or Entry type
template <typename PageTableOrEntry>
struct PageLevelGetter {
    private:
    // Helper lambda to calculate the value using C++17 if constexpr
    static constexpr u8 get_level()
    {
        if constexpr (std::is_same_v<
                          PageTableOrEntry,
                          typename detail::page_table_helpers::PageLevelTraits<1>::TableType> ||
                      std::is_same_v<
                          PageTableOrEntry,
                          typename detail::page_table_helpers::PageLevelTraits<1>::EntryType>) {
            return 1;
        } else if constexpr (std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<2>::TableType> ||
                             std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<2>::EntryType>) {
            return 2;
        } else if constexpr (std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<3>::TableType> ||
                             std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<3>::EntryType>) {
            return 3;
        } else if constexpr (std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<4>::TableType> ||
                             std::is_same_v<
                                 PageTableOrEntry, typename detail::page_table_helpers::
                                                       PageLevelTraits<4>::EntryType>) {
            return 4;
        } else {
            return 0;
        }
    }

    public:
    static constexpr u8 value = get_level();
};

inline static constexpr u8 PageLevelV = PageLevelGetter<PageTableT<1>>::value;

}  // namespace memory

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLE_HELPERS_HPP_
