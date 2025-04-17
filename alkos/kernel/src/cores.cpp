#include "hardware/cores.hpp"

#include <assert.h>
#include <extensions/debug.hpp>

void hardware::CoresController::AllocateCores(const size_t num)
{
    ASSERT_EQ(0, num_cores_, "Cores should be allocated only once at startup...");

    TODO_WHEN_VMEM_WORKS
    num_cores_ = num;
}

void hardware::CoresController::BootUpAllCores()
{
    TRACE_INFO("Booting up all cores...");

    for (size_t i = 0; i < num_cores_; i++) {
        GetCore(i).EnableCore();
    }

    TRACE_INFO("Finished booting up all cores...");
}
