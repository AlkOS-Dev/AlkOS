#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_TLB_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_TLB_HPP_

#include <extensions/expected.hpp>

#include "mem/types.hpp"

namespace Mem
{

class AddressSpace;

}

namespace arch
{

struct TlbAPI {
    /**
     * @brief Invalidates the entire TLB on the current core.
     */
    void FlushAll();

    /**
     * @brief Invalidates the TLB entry for a single page.
     * @param vaddr The virtual address of the page to invalidate.
     */
    void InvalidatePage(Mem::VPtr<void> vaddr);

    /**
     * @brief Switches the current address space, which typically involves
     * loading a new page table root into a CPU register (e.g., CR3).
     * @param as The new address space to switch to.
     */
    void SwitchAddrSpace(Mem::VPtr<Mem::AddressSpace> as);
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_TLB_HPP_
