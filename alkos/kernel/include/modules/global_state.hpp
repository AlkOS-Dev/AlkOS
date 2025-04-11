#ifndef ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_

#include <extensions/template_lib.hpp>
#include <modules/global_state_constants.hpp>

// ------------------------------
// Module
// ------------------------------

namespace internal
{
class GlobalStateModule : TemplateLib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    GlobalStateModule() noexcept;

    // ------------------------------
    // Getters
    // ------------------------------

    public:
    using SettingsT = TemplateLib::Settings<
        global_state_constants::GlobalSettingsTypes, global_state_constants::SettingsType>;

    NODISCARD FORCE_INLINE_F SettingsT& GetSettings() noexcept { return settings_; }

    // ------------------------------
    // Module fields
    // ------------------------------

    private:
    SettingsT settings_;
};
}  // namespace internal

using GlobalStateModule = TemplateLib::StaticSingleton<internal::GlobalStateModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_HPP_
