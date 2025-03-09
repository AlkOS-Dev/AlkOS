#include <stdint.h>
#include "extensions/types.hpp"

// Extern Declarations for Page Tables defined in Assembly

u64 p4_table[512];
u64 p3_table[512];
u64 p2_table[512];
u64 p1_table[512];

// Page Table Entry Flags
static constexpr u32 PRESENT_BIT       = (1 << 0);  // The page is present in memory
static constexpr u32 WRITE_BIT         = (1 << 1);  // The page is writable
static constexpr u32 USER_BIT          = (1 << 2);  // The page is accessible from user mode
static constexpr u32 WRITE_THROUGH_BIT = (1 << 3);  // Writes go directly to memory
static constexpr u32 CACHE_DISABLE_BIT = (1 << 4);  // The page is not cached
static constexpr u32 ACCESSED_BIT      = (1 << 5);  // CPU sets this bit when the page is accessed
static constexpr u32 DIRTY_BIT         = (1 << 6);  // CPU sets this bit when the page is written to
static constexpr u32 HUGE_PAGE_BIT     = (1 << 7);  // Creates a 2 MiB page in PD table
static constexpr u32 GLOBAL_BIT =
    (1 << 8);  // The page isn't flushed from the TLB on address space switch
static constexpr u32 TWO_MIB = 0x200000;
// NO_EXECUTE_BIT is not used in 32-bit PAE

/**
 * @brief Initializes the page tables for PAE paging.
 *
 * This function sets up the page tables to identity map the first 1 GiB of memory
 * using 2 MiB pages. It initializes the Page Map Level 4 (PML4), Page Directory Pointer Table
 * (PDPT), and Page Directory (PD) accordingly.
 *
 * @return int 0 because the function always succeeds
 */
int SetupPageTables()
{
    // Initialize the PML4 entry to point to the PDPT with present and writable flags
    p4_table[0] = ((u64)p3_table) | PRESENT_BIT | WRITE_BIT;

    // Initialize the PDPT entry to point to the PD with present and writable flags
    p3_table[0] = ((u64)p2_table) | PRESENT_BIT | WRITE_BIT;

    // Identity map the first 1 GiB of memory using 2 MiB pages
    for (int i = 0; i < 512; i++) {
        // Each PD entry maps a 2 MiB page
        p2_table[i] = ((u64)(TWO_MIB * i)) | PRESENT_BIT | WRITE_BIT | HUGE_PAGE_BIT;
    }

    return 0;
}
