#ifndef ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_CONSTANTS_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_CONSTANTS_HPP_

#include <extensions/template_lib.hpp>

namespace global_state_constants
{

enum class SettingsType : size_t {
    kIsDayTimeClockInUTC, /* Is true when hardware stores datetime as UTC */
    kLastSettingsType,
};

using GlobalSettingsTypes = TemplateLib::TypeList<bool /* kIsDayTimeClockInUTC */
                                                  >;

using GlobalSettingsTuple = GlobalSettingsTypes::Tuple;

static constexpr GlobalSettingsTuple kDefaultGlobalSettings{
    true, /* kIsDayTimeClockInUTC */
};

}  // namespace global_state_constants

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_GLOBAL_STATE_CONSTANTS_HPP_
