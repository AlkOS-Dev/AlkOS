#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_

#include <extensions/expected.hpp>
#include <mem/phys_ptr.hpp>
#include <mem/pmm.hpp>

#include "mem/page_map.hpp"

namespace arch
{

class PhysicalMemoryManager : public PhysicalMemoryManagerABI
{
    public:
    struct InitData {
        PhysicalPtr<byte> mem_bitmap;
        u64 total_num_pages;
        PhysicalPtr<PageMapTable<4>> pml_4_table;
    };

    //==============================================================================
    // ABI
    //==============================================================================

    std::expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest req = {});
    void Free(PhysicalPtr<Page> page);

    void Init(InitData idata);
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_
