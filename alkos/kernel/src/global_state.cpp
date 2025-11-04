#include <modules/global_state.hpp>
#include "trace_framework.hpp"

internal::GlobalStateModule::GlobalStateModule() noexcept
    : Settings_(global_state_constants::kDefaultGlobalSettings)
{
    DEBUG_INFO_GENERAL("Initialized the global state module");
}
