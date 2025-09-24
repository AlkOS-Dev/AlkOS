#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_BITMAP_PMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_BITMAP_PMM_HPP_

#include <extensions/bit_array.hpp>
#include <extensions/template_lib.hpp>

#include <mem/pmm_abi.hpp>

#include "hal/mem/pmm/bitmap_config.hpp"

namespace arch
{

// A simple PMM that takes over the bootloader's existing bitmap.
// This serves as the temporary bootstrap allocator for the kernel.
class BitmapPmm : public PhysicalMemoryManagerABI,
                  public template_lib::DelayedInitMixin<BitmapPmm, Config<BitmapPmm>>
{
    public:
    enum {
        BitMapFree      = 0,
        BitMapAllocated = 1,
    };

    BitmapPmm() = default;

    //==============================================================================
    // ABI : PhysicalMemoryManagerABI
    //==============================================================================

    std::expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest req = {});
    void Free(PhysicalPtr<Page> page, u64 num_pages);

    //==============================================================================
    // ABI : DelayedInitMixin
    //==============================================================================

    void InitImpl();

    //==============================================================================
    // Private Fields
    //==============================================================================

    private:
    BitMapView bitmap_view_{nullptr, 0};
    u64 last_alloc_idx_ = 0;
};

}  // namespace arch

#endif
