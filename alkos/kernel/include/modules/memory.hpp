#ifndef ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_

#include <extensions/template_lib.hpp>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/page_meta_table.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "modules/helpers.hpp"

namespace internal
{
class MemoryModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    explicit MemoryModule(const BootArguments& args) noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(mem, PageMetaTable);
    DEFINE_MODULE_FIELD(mem, BitmapPmm);
};
}  // namespace internal

using MemoryModule = template_lib::StaticSingleton<internal::MemoryModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_
