#ifndef ALKOS_INCLUDE_MODULES_GLOBAL_STATE_HPP_
#define ALKOS_INCLUDE_MODULES_GLOBAL_STATE_HPP_

#include <extensions/template_lib.hpp>
#include <modules/global_state_constants.hpp>

namespace internal
{
class GlobalStateModule : TemplateLib::StaticSingletonHelper
{
    protected:
    GlobalStateModule() noexcept;

    public:
    using SettingsT = TemplateLib::Settings<global_state_constants::GlobalSettingsTypes>;

    NODISCARD FORCE_INLINE_F SettingsT& GetSettings() noexcept { return settings_; }

    private:
    SettingsT settings_;
};
}  // namespace internal

using GlobalStateModule = TemplateLib::StaticSingleton<internal::GlobalStateModule>;

#endif  // ALKOS_INCLUDE_MODULES_GLOBAL_STATE_HPP_
