#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLES_LAYOUT_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLES_LAYOUT_HPP_

#include <extensions/types.hpp>
#include "defines.hpp"

namespace memory
{
enum class PageSize { Page4k, Page2M, Page1G };

// Source: Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3
// (3A, 3B & 3C): System Programming Guide

// Note: In the manual, some fields are bounded by M = MAXPHYADDR, where M is the maximum
// physical address width supported by the processor. This is 52 for 4-level paging.

static constexpr u32 kNumEntriesPerPml = 512;  ///< Number of entries in a PML table.
static constexpr u32 kAddressOffset    = 12;  ///< The offset of the address in the page table entry

// PML Flags
// This is a list of flags that can be set in the page table entries.
// They have to exist even though i used bitfields in the structs, because they are used as
// input to set the flags in the page table entries in functions like
// MapVirtualMemoryToPhysical.

// TODO: Include all of them
static constexpr u32 kPresentBit             = 1;        ///< Present bit
static constexpr u32 kWriteBit               = 1 << 1;   ///< Write bit
static constexpr u32 kUserAccessibleBit      = 1 << 2;   ///< User accessible bit
static constexpr u32 kWriteThroughCachingBit = 1 << 3;   ///< Write-through caching bit
static constexpr u32 kDisableCacheBit        = 1 << 4;   ///< Disable cache bit
static constexpr u32 kAccessedBit            = 1 << 5;   ///< Accessed bit
static constexpr u32 kDirtyBit               = 1 << 6;   ///< Dirty bit
static constexpr u32 kGlobalBit              = 1 << 8;   ///< Global bit
static constexpr u32 kPatBit                 = 1 << 12;  ///< PAT bit
static constexpr u32 kHlatRestartBit         = 1 << 13;  ///< HLAT restart bit
static constexpr u32 kHugePageBit            = 1 << 7;   ///< Huge page bit

// Table 5-15. Format of a PML4 Entry (PML4E) that References a Page-Directory-Pointer Table
struct PML4Entry_t {
    u64 present : 1;   ///< Must be 1 to reference a page-directory-pointer table
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 512-GByte region controlled
    ///< by this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 512-GByte
    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 ignored_1 : 1;     ///< Ignored (usable by software)
    u64 page_size : 1;     ///< Reserved (must be 0)
    u64 ignored_2 : 3;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;  ///< Physical address of the 4-KByte aligned page-directory-pointer table
    ///< referenced by this entry
    u64 reserved : 11;        ///< Must be 0
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 512-GByte region controlled
    ///< by this entry)
} PACK;

// Table 5-16. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that Maps a 1-GByte Page
struct PML3Entry1GB_t {
    u64 present : 1;   ///< Must be 1 to map a 1-GByte page
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 1-GByte page controlled by
    ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 1-GByte page
    ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 dirty : 1;  ///< If the memory management hardware in the processor accessed this entry
    ///< during translation of a linear address to a physical address this bit
    ///< is set
    u64 page_size : 1;     ///< Must be 1 (otherwise, this entry references a page directory)
    u64 global : 1;        ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging)
    u64 pat : 1;  ///< If PAT is supported, indirectly determines the memory type used to access
    ///< the 1-GByte page (PAT is supported on all processors with 4-level paging)
    u64 reserved_1 : 17;     ///< Must be 0
    u64 frame : 22;          ///< Physical address of the 1-GByte page referenced by this entry.
    u64 ignored_2 : 7;       ///< Ignored (usable by software)
    u64 protection_key : 4;  ///< If IA32_EFER.PKE = 1, determines the protection key of the
    ///< 1-GByte page.
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 1-GByte page controlled by
    ///< this entry)
};

// Table 5-17. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that References a Page
// Directory
struct PML3Entry_t {
    u64 present : 1;   ///< Must be 1 to reference a page directory
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 1-GByte region controlled by
    ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 1-GByte
    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 ignored_1 : 1;     ///< Ignored (usable by software)
    u64 page_size : 1;     ///< Reserved (must be 0)
    u64 ignored_2 : 3;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;  ///< Physical address of the 4-KByte aligned page directory referenced by
    ///< this entry
    u64 ignored_3 : 11;       ///< Must be 0
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 1-GByte region controlled by
    ///< this entry)
} PACK;

// Table 5-18. Format of a Page-Directory Entry (PDE) that Maps a 2-MByte Page
struct PML2Entry2MB_t {
    u64 present : 1;   ///< Must be 1 to map a 2-MByte page
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 2-MByte page controlled by
    ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 2-MByte page
    ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 dirty : 1;  ///< If the memory management hardware in the processor accessed this entry
    ///< during translation of a linear address to a physical address this bit
    ///< is set
    u64 page_size : 1;     ///< Must be 1 (otherwise, this entry references a page table)
    u64 global : 1;        ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging
    u64 pat : 1;  ///< If PAT is supported, indirectly determines the memory type used to access
    ///< the 2-MByte page (PAT is supported on all processors with 4-level paging)
    u64 reserved_1 : 8;      ///< Must be 0
    u64 frame : 31;          ///< Physical address of the 2-MByte page referenced by this entry
    u64 ignored_2 : 7;       ///< Ignored (usable by software)
    u64 protection_key : 4;  ///< If IA32_EFER.PKE = 1, determines the protection key of the
    ///< 2-MByte page
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 2-MByte page controlled by
    ///< this entry)
} PACK;

// Table 5-19. Format of a Page-Directory Entry (PDE) that References a Page Table
struct PML2Entry_t {
    u64 present : 1;   ///< Must be 1 to reference a page table
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 2-MByte region controlled by
    ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 2-MByte
    ///< region controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 ignored_1 : 1;     ///< Ignored (usable by software)
    u64 page_size : 1;     ///< Reserved (must be 0)
    u64 ignored_2 : 3;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;  ///< Physical address of the 4-KByte aligned page table referenced by this
    ///< entry
    u64 ignored_3 : 11;       ///< Ignored (usable by software)
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 2-MByte region controlled by
    ///< this entry)
} PACK;

// Table 5-20. Format of a Page-Table Entry (PTE) that Maps a 4-KByte Page
struct PML1Entry_t {
    u64 present : 1;   ///< Must be 1 to map a 4-KByte page
    u64 writable : 1;  ///< If 0, writes may not be allowed to the 4-KByte page controlled by
    ///< this entry
    u64 user_accessible : 1;  ///< If 0, user-mode accesses are not allowed to the 4-KByte page
    ///< controlled by this entry
    u64 write_through_caching : 1;  ///< Page-level write-through; indirectly determines the
    ///< memory type used to access the page
    u64 disable_cache : 1;  ///< Page-level cache disable; indirectly determines the memory type
    ///< used to access the page
    u64 accessed : 1;  ///< If the memory management hardware in the processor accessed this
    ///< entry during translation of a linear address to a physical address
    ///< this bit is set
    u64 dirty : 1;  ///< If the memory management hardware in the processor accessed this entry
    ///< during translation of a linear address to a physical address this bit
    ///< is set
    u64 pat : 1;  ///< If PAT is supported, indirectly determines the memory type used to access
    ///< the 4-KByte page (PAT is supported on all processors with 4-level paging)
    u64 global : 1;        ///< If CR4.PGE = 1, determines whether the translation is global
    u64 ignored_1 : 2;     ///< Ignored (usable by software)
    u64 hlat_restart : 1;  ///< For ordinary paging, ignored, for HLAT paging, restart (if 1,
    ///< linear address translation is restarted with ordinary paging)
    u64 frame : 40;          ///< Physical address of the 4-KByte page referenced by this entry
    u64 ignored_2 : 7;       ///< Ignored (usable by software)
    u64 protection_key : 4;  ///< If IA32_EFER.PKE = 1, determines the protection key of the
    ///< 4-KByte page
    u64 execute_disable : 1;  ///< If IA32_EFER.NXE = 1, execute-disable (if 1, instruction
    ///< fetches are not allowed from the 4-KByte page controlled by
    ///< this entry)
};

/// Type for a page table represented as an array of 512 entries, aligned to 4096 bytes.
typedef u64 PMLTable_t[kNumEntriesPerPml] __attribute__((aligned(4096)));
typedef PMLTable_t PML4_t;
typedef PMLTable_t PML3_t;
typedef PMLTable_t PML2_t;
typedef PMLTable_t PML1_t;
}  // namespace memory

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MEMORY_PAGE_TABLES_LAYOUT_HPP_
