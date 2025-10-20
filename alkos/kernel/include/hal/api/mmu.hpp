#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_

#include <extensions/expected.hpp>
#include "mem/error.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

template <typename T, typename E>
using Expected = std::expected<T, E>;
template <typename E>
using Unexpected = std::unexpected<E>;

namespace Mem
{

class AddressSpace;

}

namespace arch
{

/**
 * @brief Architecture-independent page mapping flags.
 */
enum class PageFlags : u64 {
    None           = 0ULL,
    Present        = 1ULL << 0,
    Writable       = 1ULL << 1,
    UserAccessible = 1ULL << 2,
    WriteThrough   = 1ULL << 3,
    CacheDisable   = 1ULL << 4,
    Global         = 1ULL << 8,
    NoExecute      = 1ULL << 63,
};

inline constexpr PageFlags operator|(PageFlags a, PageFlags b)
{
    return static_cast<PageFlags>(static_cast<u64>(a) | static_cast<u64>(b));
}

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
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_MMU_HPP_
