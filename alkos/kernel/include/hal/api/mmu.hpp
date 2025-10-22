#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_

#include <extensions/expected.hpp>
#include "mem/error.hpp"
#include "mem/types.hpp"

namespace Mem
{

class AddressSpace;

}

namespace arch
{

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

struct MmuAPI {
    /**
     * @brief Maps a physical page to a virtual address in a given address space.
     * This function is responsible for walking the page tables and creating entries
     * as needed.
     * @param as The target address space.
     * @param vaddr The virtual address to map. Must be page-aligned.
     * @param paddr The physical address to map to. Must be page-aligned.
     * @param flags Page protection and control flags.
     * @return Success or a memory error.
     */
    Expected<void, Mem::MemError> Map(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr, Mem::PPtr<void> paddr, PageFlags
    );

    /**
     * @brief Unmaps a virtual address, invalidating its page table entry.
     * @param as The target address space.
     * @param vaddr The virtual address to unmap.
     * @return Success or a memory error if the page is not mapped.
     */
    Expected<void, Mem::MemError> UnMap(Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr);

    /**
     * @brief Translates a virtual address to its corresponding physical address.
     * @param as The address space to perform the translation in.
     * @param vaddr The virtual address to translate.
     * @return The physical address, or a memory error if the address is not mapped.
     */
    Expected<Mem::PPtr<void>, Mem::MemError> Translate(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr
    );

    /**
     * @brief Switches the current address space, which typically involves
     * loading a new page table root into a CPU register (e.g., CR3).
     *
     * @param as The new address space to switch to.
     */
    void SwitchAddrSpace(Mem::VPtr<Mem::AddressSpace> as);
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_
