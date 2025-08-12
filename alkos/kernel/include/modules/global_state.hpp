#ifndef ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_

#include <extensions/data_structures/data_structures.hpp>
#include <extensions/template_lib.hpp>

#include "memory/cyclic_allocator.hpp"
#include "modules/global_state_constants.hpp"
#include "modules/helpers.hpp"
#include "sync/kernel/spinlock.hpp"

// ------------------------------
// Module
// ------------------------------

namespace internal
{
class GlobalStateModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    GlobalStateModule() noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    public:
    using Settings = data_structures::Settings<
        global_state_constants::GlobalSettingsTypes, global_state_constants::SettingsType>;

    using SpinlockAllocator =
        CyclicAllocator<Spinlock, global_state_constants::kStaticSpinlockAllocCount>;

    DEFINE_MODULE_FIELD(GlobalStateModule, Settings);
    DEFINE_MODULE_FIELD(GlobalStateModule, SpinlockAllocator);
};
}  // namespace internal

using GlobalStateModule = template_lib::StaticSingleton<internal::GlobalStateModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_
