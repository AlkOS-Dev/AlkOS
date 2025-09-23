#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_BUDDY_PMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_BUDDY_PMM_HPP_

#include <extensions/template_lib.hpp>

#include <mem/pmm_abi.hpp>

#include "hal/mem/pmm/buddy_config.hpp"

namespace arch
{

class BuddyPmm : public PhysicalMemoryManagerABI,
                 public template_lib::DelayedInitMixin<BuddyPmm, BuddyPmmConfig>
{
    public:
    void InitImpl() {}

    //==============================================================================
    // ABI
    //==============================================================================

    std::expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest req = {});
    void Free(PhysicalPtr<Page> page, u64 num_pages);

    private:
};

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_BUDDY_PMM_HPP_
