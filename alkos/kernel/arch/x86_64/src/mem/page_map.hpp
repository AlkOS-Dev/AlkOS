#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_MEM_PAGE_MAP_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_MEM_PAGE_MAP_HPP_

#include <extensions/bit.hpp>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include <mem/phys_ptr.hpp>

//==============================================================================
// Source: Intel® 64 and IA-32 Architectures Software Developer’s Manual,
// Volume 3 (3A, 3B & 3C): System Programming Guide
//==============================================================================
//------------------------------------------------------------------------------
// Note: In the manual, some fields are bounded by M = MAXPHYADDR, where M is
// the maximum physical address width supported by the processor. This is 52
// for 4-level paging.
//------------------------------------------------------------------------------

//==============================================================================
// Flags
//==============================================================================
//------------------------------------------------------------------------------
// This is a list of flags that can be set in the page table entries.
//------------------------------------------------------------------------------

// TODO: Properly document these flags

static constexpr u64 kPresentBit             = 1;        ///< Present bit
static constexpr u64 kWriteBit               = 1 << 1;   ///< Write bit
static constexpr u64 kUserAccessibleBit      = 1 << 2;   ///< User accessible bit
static constexpr u64 kWriteThroughCachingBit = 1 << 3;   ///< Write-through caching bit
static constexpr u64 kDisableCacheBit        = 1 << 4;   ///< Disable cache bit
static constexpr u64 kAccessedBit            = 1 << 5;   ///< Accessed bit
static constexpr u64 kDirtyBit               = 1 << 6;   ///< Dirty bit
static constexpr u64 kGlobalBit              = 1 << 8;   ///< Global bit
static constexpr u64 kPatBit                 = 1 << 12;  ///< PAT bit
static constexpr u64 kHlatRestartBit         = 1 << 13;  ///< HLAT restart bit
static constexpr u64 kHugePageBit            = 1 << 7;   ///< Huge page bit

//==============================================================================
// PageMapEntries
//==============================================================================

static constexpr bool kHugePage = true;

template <u16 kLevel, bool kIsHuge = false>
struct PageMapEntry;

template <u16 kLevel>
using PageMapTable = PageMapEntry<kLevel>[512];

enum class PageSizeTag { k4Kb, k2Mb, k1Gb };

template <PageSizeTag size>
constexpr u64 PageSize()
{
    switch (size) {
        case PageSizeTag::k4Kb:
            return 1ULL << 12;
        case PageSizeTag::k2Mb:
            return 1ULL << 21;
        case PageSizeTag::k1Gb:
            return 1ULL << 30;
        default:
            return 0;
    }
}

//------------------------------------------------------------------------------
// Level 4
//------------------------------------------------------------------------------

// Table 5-15. Format of a PML4 Entry (PML4E) that References a Page-Directory-Pointer Table
template <>
struct PageMapEntry<4> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 ignored_1 : 1;
    u64 page_size : 1;
    u64 ignored_2 : 3;
    u64 hlat_restart : 1;
    u64 frame : 40;
    u64 reserved : 11;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<PageMapTable<3>> GetNextLevelTable() const
    {
        return PhysicalPtr<PageMapTable<3>>(static_cast<u64>(frame) << 12);
    }

    void SetNextLevelTable(PhysicalPtr<PageMapTable<3>> table_addr, u64 flags)
    {
        frame = table_addr.AsUIntPtr() >> 12;
        *reinterpret_cast<u64 *>(this) |= flags;
    }
} PACK;

//------------------------------------------------------------------------------
// Level 3
//------------------------------------------------------------------------------

// Table 5-17. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that References a Page
// Directory
template <>
struct PageMapEntry<3> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 ignored_1 : 1;
    u64 page_size : 1;
    u64 ignored_2 : 3;
    u64 hlat_restart : 1;
    u64 frame : 40;
    u64 ignored_3 : 11;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<PageMapTable<2>> GetNextLevelTable() const
    {
        return PhysicalPtr<PageMapTable<2>>(static_cast<u64>(frame) << 12);
    }

    void SetNextLevelTable(PhysicalPtr<PageMapTable<2>> table_addr, u64 flags)
    {
        frame = table_addr.AsUIntPtr() >> 12;
        *reinterpret_cast<u64 *>(this) |= flags;
    }
} PACK;

// Table 5-16. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that Maps a 1-GByte Page
template <>
struct PageMapEntry<3, kHugePage> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 page_size : 1;
    u64 global : 1;
    u64 ignored_1 : 2;
    u64 hlat_restart : 1;
    u64 pat : 1;
    u64 reserved_1 : 17;
    u64 frame : 22;
    u64 ignored_2 : 7;
    u64 protection_key : 4;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<void> GetFrameAddress() const
    {
        return PhysicalPtr<void>(static_cast<u64>(frame) << 30);
    }

    void SetFrameAddress(PhysicalPtr<void> page_addr, u64 flags)
    {
        frame = page_addr.AsUIntPtr() >> 30;
        *reinterpret_cast<u64 *>(this) |= flags | kHugePageBit;
    }
} PACK;

//------------------------------------------------------------------------------
// Level 2
//------------------------------------------------------------------------------

// Table 5-19. Format of a Page-Directory Entry (PDE) that References a Page Table
template <>
struct PageMapEntry<2> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 ignored_1 : 1;
    u64 page_size : 1;
    u64 ignored_2 : 3;
    u64 hlat_restart : 1;
    u64 frame : 40;
    u64 ignored_3 : 11;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<PageMapTable<1>> GetNextLevelTable() const
    {
        return PhysicalPtr<PageMapTable<1>>(static_cast<u64>(frame) << 12);
    }

    void SetNextLevelTable(PhysicalPtr<PageMapTable<1>> table_addr, u64 flags)
    {
        frame = table_addr.AsUIntPtr() >> 12;
        *reinterpret_cast<u64 *>(this) |= flags;
    }
} PACK;

// Table 5-18. Format of a Page-Directory Entry (PDE) that Maps a 2-MByte Page
template <>
struct PageMapEntry<2, kHugePage> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 page_size : 1;
    u64 global : 1;
    u64 ignored_1 : 2;
    u64 hlat_restart : 1;
    u64 pat : 1;
    u64 reserved_1 : 8;
    u64 frame : 31;
    u64 ignored_2 : 7;
    u64 protection_key : 4;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<void> GetFrameAddress() const
    {
        return PhysicalPtr<void>(static_cast<u64>(frame) << 21);
    }

    void SetFrameAddress(PhysicalPtr<void> page_addr, u64 flags)
    {
        frame = page_addr.AsUIntPtr() >> 21;
        *reinterpret_cast<u64 *>(this) |= flags | kHugePageBit;
    }
} PACK;

//------------------------------------------------------------------------------
// Level 1
//------------------------------------------------------------------------------

// Table 5-20. Format of a Page-Table Entry (PTE) that Maps a 4-KByte Page
template <>
struct PageMapEntry<1> {
    u64 present : 1;
    u64 writable : 1;
    u64 user_accessible : 1;
    u64 write_through_caching : 1;
    u64 disable_cache : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 pat : 1;
    u64 global : 1;
    u64 ignored_1 : 2;
    u64 hlat_restart : 1;
    u64 frame : 40;
    u64 ignored_2 : 7;
    u64 protection_key : 4;
    u64 execute_disable : 1;

    // Accessors

    NODISCARD bool IsPresent() const { return present; }

    NODISCARD PhysicalPtr<void> GetFrameAddress() const
    {
        return PhysicalPtr<void>(static_cast<u64>(frame) << 12);
    }

    void SetFrameAddress(PhysicalPtr<void> page_addr, u64 flags)
    {
        frame = page_addr.AsUIntPtr() >> 12;
        *reinterpret_cast<u64 *>(this) |= flags;
    }
} PACK;

//==============================================================================
// Static Validation
//==============================================================================

template <u16 kLevel, bool kIsHuge>
struct PageMapEntry {
    static_assert(kLevel != 4 || !kIsHuge, "4th level page map entries cannot be huge");
    static_assert(kLevel != 1 || !kIsHuge, "1st level page map entries cannot be huge");
};

// All entries must be 8 bytes
static_assert(sizeof(PageMapEntry<4>) == 8, "PML4E must be 8 bytes");
static_assert(sizeof(PageMapEntry<3>) == 8, "PDPTE (to PD) must be 8 bytes");
static_assert(sizeof(PageMapEntry<3, kHugePage>) == 8, "PDPTE (to 1GB page) must be 8 bytes");
static_assert(sizeof(PageMapEntry<2>) == 8, "PDE (to PT) must be 8 bytes");
static_assert(sizeof(PageMapEntry<2, kHugePage>) == 8, "PDE (to 2MB page) must be 8 bytes");
static_assert(sizeof(PageMapEntry<1>) == 8, "PTE (to 4KB page) must be 8 bytes");

// All tables must be 4KB
static_assert(sizeof(PageMapTable<4>) == 4096, "PML4 table must be 4KB");
static_assert(sizeof(PageMapTable<3>) == 4096, "PDP table must be 4KB");
static_assert(sizeof(PageMapTable<2>) == 4096, "PD table must be 4KB");
static_assert(sizeof(PageMapTable<1>) == 4096, "PT table must be 4KB");

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_MEM_PAGE_MAP_HPP_
