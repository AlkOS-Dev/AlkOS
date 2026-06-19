// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <acpi/acpi.hpp>

#include <internal/formats.hpp>
#include "trace_framework.hpp"

#include <uacpi/event.h>
#include <uacpi/uacpi.h>

void ACPI::ACPIController::EarlyInit(const BootArguments &args)
{
    TODO_WHEN_VMEM_WORKS
    DEBUG_INFO_ACPI("Early ACPI initialization...");

    DEBUG_INFO_ACPI("Getting RSDP from boot arguments...");
    R_ASSERT_NOT_NULL(
        args.rsdp,
        "RSDP address not provided in boot arguments, only platforms with ACPI supported..."
    );

    RsdpAddress_ = args.rsdp;
    DEBUG_INFO_ACPI("RSDP address: 0x%016X", RsdpAddress_);

    /* Load all tables, bring the event subsystem online, and enter ACPI mode */
    uacpi_status ret = uacpi_initialize(0);
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_initialize error: %s", uacpi_status_to_string(ret));
}

void ACPI::ACPIController::Init()
{
    DEBUG_INFO_ACPI("Full ACPI initialization...");

    /* Load the AML namespace */
    uacpi_status ret = uacpi_namespace_load();
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_namespace_load error: %s", uacpi_status_to_string(ret));

    /* Initialize the namespace */
    ret = uacpi_namespace_initialize();
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));

    /*
     * Tell uACPI that we have marked all GPEs we wanted for wake. This is needed to
     * let uACPI enable all unmarked GPEs that have a corresponding AML handler.
     */
    ret = uacpi_finalize_gpe_initialization();
    R_ASSERT_ACPI_SUCCESS(
        ret, "uacpi_finalize_gpe_initialization error: %s", uacpi_status_to_string(ret)
    );
}
