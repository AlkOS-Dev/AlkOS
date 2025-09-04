#ifndef ALKOS_BOOT_LIB_MEM_PAGE_MAP_HPP_
#define ALKOS_BOOT_LIB_MEM_PAGE_MAP_HPP_

#include <extensions/defines.hpp>

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

// TODO: Include all of them
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
struct PageMapEntry {
    static_assert(kLevel != 4 || !kIsHuge, "4th level page map entries cannot be huge");
    static_assert(kLevel != 1 || !kIsHuge, "1st level page map entries cannot be huge");
};

//------------------------------------------------------------------------------
// Level 4
//------------------------------------------------------------------------------

// Table 5-15. Format of a PML4 Entry (PML4E) that References a Page-Directory-Pointer Table
template <>
struct PageMapEntry<4> {
    u64 present : 1;   ///< Must be 1 to reference a page-directory-pointer table
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 512-GByte region controlled
                       ///< by this entry
    u64 user_accessible : 1;        ///< If 0, user-mode accesses are not allowed to the 512-GByte
                                    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 ignored_1 : 1;      ///< Ignored (usable by software)
    u64 page_size : 1;      ///< Reserved (must be 0)
    u64 ignored_2 : 3;      ///< Ignored (usable by software)
    u64 hlat_restart : 1;   ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                            ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;     ///< Physical address of the 4-KByte aligned page-directory-pointer table
                        ///< referenced by this entry
    u64 reserved : 11;  ///< Must be 0
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 512-GByte region controlled
                              ///< by this entry)
} PACK;

//------------------------------------------------------------------------------
// Level 3
//------------------------------------------------------------------------------

// Table 5-17. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that References a Page
// Directory
template <>
struct PageMapEntry<3> {
    u64 present : 1;   ///< Must be 1 to reference a page directory
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 1-GByte region controlled by
                       ///< this entry
    u64 user_accessible : 1;        ///< If 0, user-mode accesses are not allowed to the 1-GByte
                                    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 ignored_1 : 1;      ///< Ignored (usable by software)
    u64 page_size : 1;      ///< Reserved (must be 0)
    u64 ignored_2 : 3;      ///< Ignored (usable by software)
    u64 hlat_restart : 1;   ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                            ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;      ///< Physical address of the 4-KByte aligned page directory referenced by
                         ///< this entry
    u64 ignored_3 : 11;  ///< Must be 0
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 1-GByte region controlled by
                              ///< this entry)
} PACK;

// Table 5-16. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that Maps a 1-GByte Page
template <>
struct PageMapEntry<3, kHugePage> {
    u64 present : 1;          ///< Must be 1 to map a 1-GByte page
    u64 writable : 1;         ///< If 0, writes may not be allowed to the 1-GByte page controlled by
                              ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 1-GByte page
                              ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 dirty : 1;      ///< If the memory management hardware in the processor accessed this entry
                        ///< during translation of a linear address to a physical address this bit
                        ///< is set
    u64 page_size : 1;  ///< Must be 1 (otherwise, this entry references a page directory)
    u64 global : 1;     ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;  ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                           ///< linear address translation is restarted with ordinary paging)
    u64 pat : 1;  ///< If PAT is supported, indirectly determines the memory type used to access
                  ///< the 1-GByte page (PAT is supported on all processors with 4-level paging)
    u64 reserved_1 : 17;      ///< Must be 0
    u64 frame : 22;           ///< Physical address of the 1-GByte page referenced by this entry.
    u64 ignored_2 : 7;        ///< Ignored (usable by software)
    u64 protection_key : 4;   ///< If IA32_EFER.PKE = 1, determines the protection key of the
                              ///< 1-GByte page.
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 1-GByte page controlled by
                              ///< this entry)
} PACK;

//------------------------------------------------------------------------------
// Level 2
//------------------------------------------------------------------------------

// Table 5-19. Format of a Page-Directory Entry (PDE) that References a Page Table
template <>
struct PageMapEntry<2> {
    u64 present : 1;   ///< Must be 1 to reference a page table
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 2-MByte region controlled by
                       ///< this entry
    u64 user_accessible : 1;        ///< If 0, user-mode accesses are not allowed to the 2-MByte
                                    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 ignored_1 : 1;      ///< Ignored (usable by software)
    u64 page_size : 1;      ///< Reserved (must be 0)
    u64 ignored_2 : 3;      ///< Ignored (usable by software)
    u64 hlat_restart : 1;   ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                            ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;      ///< Physical address of the 4-KByte aligned page table referenced by this
                         ///< entry
    u64 ignored_3 : 11;  ///< Ignored (usable by software)
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 2-MByte region controlled by
                              ///< this entry)
} PACK;

// Table 5-18. Format of a Page-Directory Entry (PDE) that Maps a 2-MByte Page
template <>
struct PageMapEntry<2, kHugePage> {
    u64 present : 1;          ///< Must be 1 to map a 2-MByte page
    u64 writable : 1;         ///< If 0, writes may not be allowed to the 2-MByte page controlled by
                              ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 2-MByte page
                              ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 dirty : 1;      ///< If the memory management hardware in the processor accessed this entry
                        ///< during translation of a linear address to a physical address this bit
                        ///< is set
    u64 page_size : 1;  ///< Must be 1 (otherwise, this entry references a page table)
    u64 global : 1;     ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;  ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                           ///< linear address translation is restarted with ordinary paging
    u64 pat : 1;  ///< If PAT is supported, indirectly determines the memory type used to access
                  ///< the 2-MByte page (PAT is supported on all processors with 4-level paging)
    u64 reserved_1 : 8;       ///< Must be 0
    u64 frame : 31;           ///< Physical address of the 2-MByte page referenced by this entry
    u64 ignored_2 : 7;        ///< Ignored (usable by software)
    u64 protection_key : 4;   ///< If IA32_EFER.PKE = 1, determines the protection key of the
                              ///< 2-MByte page
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 2-MByte page controlled by
                              ///< this entry)
} PACK;

//------------------------------------------------------------------------------
// Level 1
//------------------------------------------------------------------------------

// Table 5-20. Format of a Page-Table Entry (PTE) that Maps a 4-KByte Page
template <>
struct PageMapEntry<1> {
    u64 present : 1;          ///< Must be 1 to map a 4-KByte page
    u64 writable : 1;         ///< If 0, writes may not be allowed to the 4-KByte page controlled by
                              ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 4-KByte page
                              ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
                                    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
                            ///< used to access the page
    u64 accessed : 1;       ///< If the memory management hardware in the processor accessed this
                            ///< entry during translation of a linear address to a physical address
                            ///< this bit is set
    u64 dirty : 1;   ///< If the memory management hardware in the processor accessed this entry
                     ///< during translation of a linear address to a physical address this bit
                     ///< is set
    u64 pat : 1;     ///< If PAT is supported, indirectly determines the memory type used to access
                     ///< the 4-KByte page (PAT is supported on all processors with 4-level paging)
    u64 global : 1;  ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;        ///< Ignored (usable by software)
    u64 hlat_restart : 1;     ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
                              ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;           ///< Physical address of the 4-KByte page referenced by this entry
    u64 ignored_2 : 7;        ///< Ignored (usable by software)
    u64 protection_key : 4;   ///< If IA32_EFER.PKE = 1, determines the protection key of the
                              ///< 4-KByte page
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
                              ///< fetches are not allowed from the 4-KByte page controlled by
                              ///< this entry)
} PACK;

//==============================================================================
// PageMapTable
//==============================================================================

template <u16 kLevel>
using PageMapTable = PageMapEntry<kLevel>[512];

//==============================================================================
// Static Validation
//==============================================================================

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

#endif  // ALKOS_BOOT_LIB_MEM_PAGE_MAP_HPP_
