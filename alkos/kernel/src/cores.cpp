#include "hardware/cores.hpp"

#include <assert.h>
#include <extensions/debug.hpp>

void hardware::CoresController::BootUpAllCores()
{
    TRACE_INFO("Booting up all cores...");

    // for (auto& core : core_table_) {
    //     core.EnableCore();
    // }

    TRACE_INFO("Finished booting up all cores...");
}
