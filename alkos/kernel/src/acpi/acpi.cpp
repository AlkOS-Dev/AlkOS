#include <acpi/acpi.hpp>

/* Internal includes */
#include <arch_utils.hpp>
#include <extensions/debug.hpp>

/* External includes */
#include <uacpi/event.h>
#include <uacpi/sleep.h>
#include <uacpi/uacpi.h>

#include <todo.hpp>

int ACPI::Init()
{
    /* Load all tables, bring the event subsystem online, and enter ACPI mode */
    uacpi_status ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret)) {
        TRACE_ERROR("uacpi_initialize error: %s", uacpi_status_to_string(ret));
        return -1;
    }

    /* Load the AML namespace */
    ret = uacpi_namespace_load();
    if (uacpi_unlikely_error(ret)) {
        TRACE_ERROR("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
        return -1;
    }

    /* Initialize the namespace */
    ret = uacpi_namespace_initialize();
    if (uacpi_unlikely_error(ret)) {
        TRACE_ERROR("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
        return -1;
    }

    /*
     * Tell uACPI that we have marked all GPEs we wanted for wake. This is needed to
     * let uACPI enable all unmarked GPEs that have a corresponding AML handler.
     */
    ret = uacpi_finalize_gpe_initialization();
    if (uacpi_unlikely_error(ret)) {
        TRACE_ERROR("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
        return -1;
    }

    return 0;
}

bool ACPI::Shutdown()
{
    /*
     * Prepare the system for shutdown.
     * This will run the \_PTS & \_SST methods, if they exist, as well as
     * some work to fetch the \_S5 and \_S0 values to make system wake
     * possible later on.
     */
    uacpi_status ret = uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S5);
    if (uacpi_unlikely_error(ret)) {
        TRACE_ERROR("failed to prepare for sleep: %s", uacpi_status_to_string(ret));
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
        TRACE_ERROR("failed to enter sleep: %s", uacpi_status_to_string(ret));
        EnableHardwareInterrupts();
        return false;
    }

    KernelPanic("Shutdown failed");
}

bool ACPI::Reboot()
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
        TRACE_ERROR("failed to prepare for sleep: %s", uacpi_status_to_string(ret));
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
        TRACE_ERROR("failed to reboot: %s", uacpi_status_to_string(ret));
        EnableHardwareInterrupts();
        return false;
    }

    KernelPanic("Reboot failed");
}

uacpi_table ACPI::GetRSDT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("RSDT", &table);
    return table;
}

uacpi_table ACPI::GetXSDT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("XSDT", &table);
    return table;
}

uacpi_table ACPI::GetFADT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("FACP", &table);
    return table;
}

uacpi_table ACPI::GetMADT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("APIC", &table);
    return table;
}

uacpi_table ACPI::GetDSDT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("DSDT", &table);
    return table;
}

uacpi_table ACPI::GetSSDT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("SSDT", &table);
    return table;
}

uacpi_table ACPI::GetSRAT()
{
    uacpi_table table;
    uacpi_table_find_by_signature("SRAT", &table);
    return table;
}
