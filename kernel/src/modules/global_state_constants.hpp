#ifndef KERNEL_SRC_MODULES_GLOBAL_STATE_CONSTANTS_HPP_
#define KERNEL_SRC_MODULES_GLOBAL_STATE_CONSTANTS_HPP_

#include <template_lib.hpp>

namespace global_state_constants
{
static constexpr size_t kStaticSpinlockAllocCount = 512;

// ------------------------------
// Settings constants
// ------------------------------

enum class SettingsType : size_t {
    kLastSettingsType,
};

using GlobalSettingsTypes = template_lib::TypeList<bool /* kIsDayTimeClockInUTC */>;

using GlobalSettingsTuple = GlobalSettingsTypes::Tuple;

static constexpr GlobalSettingsTuple kDefaultGlobalSettings{
    true, /* kIsDayTimeClockInUTC */
};
}  // namespace global_state_constants

#endif  // KERNEL_SRC_MODULES_GLOBAL_STATE_CONSTANTS_HPP_
