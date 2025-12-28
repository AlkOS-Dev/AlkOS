#ifndef KERNEL_SRC_HAL_API_MMU_HPP_
#define KERNEL_SRC_HAL_API_MMU_HPP_

#include <concepts.hpp>
#include <expected.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"

namespace arch
{

using std::expected;
using std::unexpected;

/**
 * @brief Architecture-independent page mapping flags.
 */
struct PageFlags {
    bool Present : 1;
    bool Writable : 1;
    bool UserAccessible : 1;
    bool WriteThrough : 1;
    bool CacheDisable : 1;
    bool Global : 1;
    bool NoExecute : 1;
};

/**
 * @brief Concept defining the context required by the MMU for memory operations.
 *
 * This context abstracts the management of page table pages (allocation/freeing)
 * and the tracking of their usage (reference counting). This allows the
 * architecture-specific MMU code to be decoupled from the kernel's memory
 * management implementation.
 */
template <typename T>
concept MmuContext = requires(T ctx, uint8_t level, Mem::PPtr<void> table) {
    /**
     * @brief Allocates a zeroed physical page for a translation table at the specified level.
     * @param level The level of the table being allocated (e.g., 3 for PDPT in x86_64).
     * @return Physical pointer to the allocated page, or error.
     */
    { ctx.AllocateTable(level) } -> std::same_as<expected<Mem::PPtr<void>, Mem::MemError>>;

    /**
     * @brief Frees a translation table page.
     * @param table Physical pointer to the table to free.
     * @param level The level of the table.
     */
    { ctx.FreeTable(table, level) } -> std::same_as<void>;

    /**
     * @brief Called when a new entry is inserted into a table (increasing its usage).
     * @param table Physical pointer to the table.
     */
    { ctx.IncreaseUsage(table) } -> std::same_as<void>;

    /**
     * @brief Called when an entry is removed from a table (decreasing its usage).
     * @param table Physical pointer to the table.
     * @return true if the table is now empty and should be freed/removed from its parent.
     */
    { ctx.DecreaseUsage(table) } -> std::same_as<bool>;
};

/**
 * @brief Concept for a visitor function used to traverse page tables.
 */
template <typename Func>
concept TableVisitor = requires(Func f, Mem::PPtr<void> table, uint8_t level) {
    { f(table, level) } -> std::same_as<void>;
};

struct MmuAPI {
    /**
     * @brief Maps a physical page to a virtual address in the specified page table hierarchy.
     *
     * @param ctx The MMU context for managing table allocations and metadata.
     * @param root The physical address of the root page table (e.g., PML4).
     * @param vaddr The virtual address to map. Must be page-aligned.
     * @param paddr The physical address to map to. Must be page-aligned.
     * @param flags Page protection and control flags.
     */
    template <MmuContext Context>
    expected<void, Mem::MemError> Map(
        Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr, Mem::PPtr<void> paddr,
        PageFlags flags
    );

    /**
     * @brief Unmaps a virtual address from the specified page table hierarchy.
     *
     * @param ctx The MMU context for managing table freeing and metadata.
     * @param root The physical address of the root page table.
     * @param vaddr The virtual address to unmap.
     */
    template <MmuContext Context>
    expected<void, Mem::MemError> Unmap(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr);

    /**
     * @brief Updates flags for an existing mapping.
     */
    expected<void, Mem::MemError> SetPageFlags(
        Mem::PPtr<void> root, Mem::VPtr<void> vaddr, PageFlags flags
    );

    /**
     * @brief Translates a virtual address to physical using the specified root.
     * @param ctx The MMU context (unused for simple translation but kept for API consistency if
     * needed).
     */
    template <MmuContext Context>
    expected<Mem::PPtr<void>, Mem::MemError> Translate(
        Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr
    );

    /**
     * @brief Switches the active page table root (e.g., loads CR3).
     */
    void SwitchRoot(Mem::PPtr<void> root);

    /**
     * @brief Walks the page table hierarchy and calls the visitor for each present table.
     * Useful for reconstructing metadata or debugging.
     */
    template <TableVisitor Visitor>
    void VisitTables(Mem::PPtr<void> root, Visitor visitor);
};

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_MMU_HPP_
