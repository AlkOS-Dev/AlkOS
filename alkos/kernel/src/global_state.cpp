#include <extensions/debug.hpp>
#include <modules/global_state.hpp>

internal::GlobalStateModule::GlobalStateModule() noexcept
    : settings_(global_state_constants::kDefaultGlobalSettings)
{
    TRACE_SUCCESS("Initialized the global state module");
}
