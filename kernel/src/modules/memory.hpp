#ifndef KERNEL_SRC_MODULES_MEMORY_HPP_
#define KERNEL_SRC_MODULES_MEMORY_HPP_

#include <template_lib.hpp>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "hal/mmu.hpp"
#include "hal/tlb.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/phys/mngr/slab.hpp"
#include "mem/virt/vmm.hpp"
#include "modules/hardware.hpp"
#include "modules/helpers.hpp"

namespace internal
{
class MemoryModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    explicit MemoryModule(const BootArguments &args) noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    using KernelAddressSpace = Mem::AddressSpace;

    DEFINE_MODULE_FIELD(Mem, PageMetaTable);
    DEFINE_MODULE_FIELD(Mem, BitmapPmm);
    DEFINE_MODULE_FIELD(Mem, BuddyPmm);
    DEFINE_MODULE_FIELD(Mem, SlabAllocator);
    DEFINE_MODULE_FIELD(Mem, Vmm);
    DEFINE_MODULE_FIELD(hal, Tlb);
    DEFINE_MODULE_FIELD(hal, Mmu);
    DEFINE_MODULE_FIELD(MemoryModule, KernelAddressSpace);

    public:
    void RegisterPageFault(HardwareModule &hw);
};
}  // namespace internal

using MemoryModule = template_lib::StaticSingleton<internal::MemoryModule>;

#endif  // KERNEL_SRC_MODULES_MEMORY_HPP_
