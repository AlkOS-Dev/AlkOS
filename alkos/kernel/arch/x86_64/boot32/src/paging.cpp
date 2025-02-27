#include <assert.h>
#include <stdint.h>
#include <paging.hpp>
#include <types.hpp>
#include "debug.hpp"
#include "memory.h"

// Extern Declarations for Page Tables defined in Assembly

u64 p4_table_loader[512] __attribute__((aligned(4096)));
u64 p3_table_loader[512] __attribute__((aligned(4096)));
u64 p2_table_loader[512] __attribute__((aligned(4096)));
u64 p1_table_loader[512] __attribute__((aligned(4096)));

u64 p3_table_kernel_identity[512] __attribute__((aligned(4096)));
u64 p2_table_kernel_identity[512] __attribute__((aligned(4096)));
u64 p1_table_kernel_identity[512] __attribute__((aligned(4096)));

u64 p3_table_kernel_higher_half[512] __attribute__((aligned(4096)));
u64 p2_table_kernel_higher_half[512] __attribute__((aligned(4096)));
u64 p1_table_kernel_higher_half[512] __attribute__((aligned(4096)));

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

void ClearPageTables()
{
    memset(p4_table_loader, 0, sizeof(p4_table_loader));
    memset(p3_table_loader, 0, sizeof(p3_table_loader));
    memset(p2_table_loader, 0, sizeof(p2_table_loader));
    memset(p1_table_loader, 0, sizeof(p1_table_loader));

    memset(p3_table_kernel_identity, 0, sizeof(p3_table_kernel_identity));
    memset(p2_table_kernel_identity, 0, sizeof(p2_table_kernel_identity));
    memset(p1_table_kernel_identity, 0, sizeof(p1_table_kernel_identity));

    memset(p3_table_kernel_higher_half, 0, sizeof(p3_table_kernel_higher_half));
    memset(p2_table_kernel_higher_half, 0, sizeof(p2_table_kernel_higher_half));
    memset(p1_table_kernel_higher_half, 0, sizeof(p1_table_kernel_higher_half));
}

/**
 * @brief Initializes the page tables for PAE paging.
 *
 * This function sets up the page tables to identity map the first 1 GiB of memory
 * using 2 MiB pages. It initializes the Page Map Level 4 (PML4), Page Directory Pointer Table
 * (PDPT), and Page Directory (PD) accordingly.
 */
void IdentityMapFirst4GbOfMemory()
{
    for (int i = 0; i < 512; i++) {
        MapVirtualMemoryToPhysical(
            TWO_MIB * i, 0, TWO_MIB * i, 0, PRESENT_BIT | WRITE_BIT | HUGE_PAGE_BIT,
            p4_table_loader, p3_table_loader, p2_table_loader, nullptr
        );
    }
}

/**
 * @brief Identity maps the kernel memory.
 * This is a helper function to allow the elf loader to then move the kernel to the higher half.
 */
void IndentityMapKernelMemory(u64 kernel_start, u64 kernel_end)
{
    //    for (u64 i = kernel_start; i < kernel_end; i += TWO_MIB) {
    //        MapVirtualMemoryToPhysical(
    //            i, i, PRESENT_BIT | WRITE_BIT | HUGE_PAGE_BIT,
    //            p4_table_loader,
    //            p3_table_kernel_identity,
    //            p2_table_kernel_identity, nullptr
    //        );
    //    }
}

/**
 * @brief Maps the kernel memory to the higher half of the virtual address space.
 *
 * @param kernel_start The start of the kernel memory (identity mapped).
 * @param kernel_end The end of the kernel memory (identity mapped).
 *
 * @note This function assumes that the kernel memory is identity mapped.
 */
void MapKernelMemoryToHigherHalf(u64 kernel_start, u64 kernel_end)
{
    R_ASSERT((kernel_start & 0xFFF) == 0);  // Virtual address must be page aligned

    //    for (u64 i = kernel_start; i < kernel_end; i += TWO_MIB) {
    //        u64 KernelVirtualAddress  = i - kernel_start + kHigherHalfOffset;
    //        u64 KernelPhysicalAddress = i;
    //        MapVirtualMemoryToPhysical(
    //            KernelVirtualAddress, KernelPhysicalAddress, PRESENT_BIT | WRITE_BIT |
    //            HUGE_PAGE_BIT, p4_table_loader, p3_table_kernel_higher_half,
    //            p2_table_kernel_higher_half, nullptr
    //        );
    //    }
}

/**
 * @brief Maps a virtual address to a physical address using the provided page tables.
 *
 * This function sets up the necessary page table entries to map a given virtual address
 * to a specified physical address with the provided flags.
 * It uses the provided Page Directory Pointer Table (PDPT), Page Directory (PD), and Page Table
 * (PT) And the hardcoded PML4 table to set up the page tables.
 *
 * This is for mapping both the loader memory at it's startup, and the kernel initial memory.
 *
 * @param virtual_address The virtual address to map.
 * @param physical_address The physical address to map to.
 * @param flags The flags to set for the page table entry.
 * @param p3_table The Page Directory Pointer Table to use.
 * @param p2_table The Page Directory to use.
 * @param p1_table The Page Table to use.
 */
void MapVirtualMemoryToPhysical(
    u32 virtual_address_lower, u32 virtual_address_upper, u32 physical_address_lower,
    u32 physical_address_upper, u32 flags, u64* p4_table, u64* p3_table, u64* p2_table,
    u64* p1_table
)
{
    R_ASSERT((virtual_address_lower & 0xFFF) == 0);   // Virtual address must be page aligned
    R_ASSERT((physical_address_lower & 0xFFF) == 0);  // Physical address must be page aligned

    //    TRACE_INFO(
    //        "Mapping virtual address upper: 0x%X lower: 0x%X to physical address upper: 0x%X
    //        lower: 0x%X with flags 0x%X", virtual_address_upper, virtual_address_lower,
    //        physical_address_upper, physical_address_lower, flags
    //    );

    // Calculate the indexes for each level of the page table
    u32 p4_index = (virtual_address_upper >> (39 - 32)) & 0x1FF;
    u32 p3_index = (virtual_address_lower >> 30) & 0x1FF;
    u32 p2_index = (virtual_address_lower >> 21) & 0x1FF;
    u32 p1_index = (virtual_address_lower >> 12) & 0x1FF;

    // Ensure PML4 entry points to the correct PDPT
    if (!(p4_table[p4_index] & PRESENT_BIT)) {
        p4_table[p4_index] = reinterpret_cast<u64>(p3_table) | PRESENT_BIT | WRITE_BIT;
    }

    // Ensure PDPT entry points to the correct PD
    if (!(p3_table[p3_index] & PRESENT_BIT)) {
        p3_table[p3_index] = reinterpret_cast<u64>(p2_table) | PRESENT_BIT | WRITE_BIT;
    }

    if (flags & HUGE_PAGE_BIT) {
        p2_table[p2_index] = physical_address_lower |
                             static_cast<u64>(physical_address_upper) << 32 | PRESENT_BIT |
                             WRITE_BIT | flags;
        return;
    }

    // Ensure PD entry points to the correct PT
    if (!(p2_table[p2_index] & PRESENT_BIT)) {
        p2_table[p2_index] = reinterpret_cast<u64>(p1_table) | PRESENT_BIT | WRITE_BIT;
    }

    p1_table[p1_index] = physical_address_lower | static_cast<u64>(physical_address_upper) << 32 |
                         PRESENT_BIT | WRITE_BIT | flags;
}
