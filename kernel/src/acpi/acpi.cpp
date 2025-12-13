#include <acpi/acpi.hpp>

#include <internal/formats.hpp>
#include "trace_framework.hpp"

/* External includes */
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

int ACPI::ACPIController::Init(const BootArguments &args)
{
    TODO_WHEN_VMEM_WORKS
    DEBUG_INFO_GENERAL("ACPI initialization...");

    DEBUG_INFO_GENERAL("Getting RSDP from boot arguments...");
    R_ASSERT_NOT_NULL(
        args.rsdp,
        "RSDP address not provided in boot arguments, only platforms with ACPI supported..."
    );

    RsdpAddress_ = args.rsdp;
    DEBUG_INFO_GENERAL("RSDP address: 0x%016p", RsdpAddress_);

    /* Load all tables, bring the event subsystem online, and enter ACPI mode */
    uacpi_status ret = uacpi_initialize(0);
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_initialize error: %s", uacpi_status_to_string(ret));

    // TODO: When OSL finished
    // /* Load the AML namespace */
    // ret = uacpi_namespace_load();
    // if (uacpi_unlikely_error(ret)) {
    //     TRACE_ERROR("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }
    //
    // /* Initialize the namespace */
    // ret = uacpi_namespace_initialize();
    // if (uacpi_unlikely_error(ret)) {
    //     TRACE_ERROR("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }
    //
    // /*
    //  * Tell uACPI that we have marked all GPEs we wanted for wake. This is needed to
    //  * let uACPI enable all unmarked GPEs that have a corresponding AML handler.
    //  */
    // ret = uacpi_finalize_gpe_initialization();
    // if (uacpi_unlikely_error(ret)) {
    //     TRACE_ERROR("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }

    return 0;
}
