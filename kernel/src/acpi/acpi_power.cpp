#include <acpi/acpi_power.hpp>

#include <todo.hpp>
#include "hal/panic.hpp"
#include "trace_framework.hpp"

#include <uacpi/sleep.h>

// TODO SHOULD BE THROUGH HAL
#include <cpu/utils.hpp>

bool ACPI::SystemShutdown()
{
    /*
     * Prepare the system for shutdown.
     * This will run the \_PTS & \_SST methods, if they exist, as well as
     * some work to fetch the \_S5 and \_S0 values to make system wake
     * possible later on.
     */
    uacpi_status ret = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        TRACE_FATAL_GENERAL("failed to prepare for sleep: %s", uacpi_status_to_string(ret));
        return false;
    }

    /*
     * This is where we disable interrupts to prevent anything from
     * racing with our shutdown sequence below.
     */
    BlockHardwareInterrupts();

    /*
     * Actually do the work of entering the sleep state by writing to the hardware
     * registers with the values we fetched during preparation.
     * This will also disable runtime events and enable only those that are
     * needed for wake.
     */
    ret = uacpi_enter_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        TRACE_FATAL_GENERAL("failed to enter sleep: %s", uacpi_status_to_string(ret));
        EnableHardwareInterrupts();
        return false;
    }

    hal::KernelPanic("Shutdown failed");
    return false;
}

bool ACPI::SystemReboot()
{
    /*
     * Prepare the system for reboot.
     * This will run the \_PTS & \_SST methods, if they exist, as well as
     * some work to fetch the \_S5 and \_S0 values to make system wake
     * possible later on.
     */
    TODO_WHEN_VMEM_WORKS
    uacpi_status ret = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        TRACE_FATAL_GENERAL("failed to prepare for sleep: %s", uacpi_status_to_string(ret));
        return false;
    }

    /*
     * This is where we disable interrupts to prevent anything from
     * racing with our reboot sequence below.
     */
    BlockHardwareInterrupts();

    /* Attempt to reboot via ACPI */
    ret = uacpi_reboot();
    if (uacpi_unlikely_error(ret)) {
        TRACE_FATAL_GENERAL("failed to reboot: %s", uacpi_status_to_string(ret));
        EnableHardwareInterrupts();
        return false;
    }

    hal::KernelPanic("Reboot failed");
    return false;
}
