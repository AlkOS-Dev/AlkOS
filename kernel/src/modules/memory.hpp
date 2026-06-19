// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MODULES_MEMORY_HPP_
#define KERNEL_SRC_MODULES_MEMORY_HPP_

#include <template_lib.hpp>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "hal/io.hpp"
#include "hal/mmu.hpp"
#include "hal/tlb.hpp"
#include "mem/heap.hpp"
#include "mem/mmu/contexts.hpp"
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

    DEFINE_MODULE_FIELD(Mem, PageMetaTable);
    DEFINE_MODULE_FIELD(Mem, BitmapPmm);
    DEFINE_MODULE_FIELD(Mem, BuddyPmm);
    DEFINE_MODULE_FIELD(Mem, SlabAllocator);
    DEFINE_MODULE_FIELD(Mem, Heap);
    DEFINE_MODULE_FIELD(Mem, Vmm);
    DEFINE_MODULE_FIELD(hal, Tlb);
    DEFINE_MODULE_FIELD(hal, Mmu);
    DEFINE_MODULE_FIELD(Mem, KernelMmuContext);

    public:
    void RegisterPageFault(HardwareModule &hw);

    Mem::AddressSpace &GetKernelAddressSpace() { return Vmm_.GetKernelAddressSpace(); }

    /// @brief Register initial Virtual Memory Areas (VMAs) so the VMM is aware of them.
    /// These areas were set up by the bootloader but are invisible to the generic VMM until
    /// registered.
    /// @note This can't be done in Init() because it requires MemoryModule to be fully initialized.
    void RegisterKernelVMAreas(const BootArguments &args);
};
}  // namespace internal

using MemoryModule = template_lib::StaticSingleton<internal::MemoryModule>;

#endif  // KERNEL_SRC_MODULES_MEMORY_HPP_
