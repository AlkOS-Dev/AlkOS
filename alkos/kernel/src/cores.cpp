#include "hardware/cores.hpp"

#include <assert.h>
#include <extensions/debug.hpp>

void hardware::CoresController::AllocateCores(const size_t num_cores)
{
    R_ASSERT_NOT_ZERO(num_cores, "No cores where found!");
    ASSERT_ZERO(num_cores_, "Cores should be allocated only once at startup...");

    TODO_WHEN_VMEM_WORKS
    num_cores_ = num_cores;
}

void hardware::CoresController::BootUpAllCores()
{
    TRACE_INFO("Booting up all cores...");

    for (size_t i = 0; i < num_cores_; i++) {
        GetCore(i).EnableCore();
    }

    TRACE_INFO("Finished booting up all cores...");
}
