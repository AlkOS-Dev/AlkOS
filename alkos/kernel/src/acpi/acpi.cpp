#include <acpi/acpi.hpp>

#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>

// TODO
#include <include/multiboot2/multiboot2.h>
#include <include/multiboot2/multiboot_info.hpp>

/* External includes */
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

static Multiboot::TagNewAcpi* FindAcpiTag(u64 multiboot_info_addr)
{
    // TODO: 1. Multiboot Tags should be in both the arch and the kernel, so maybe move to libk?
    // TODO: 2. This should probably recieve a MultibootInfo object instead of an address.
    TRACE_INFO("Finding ACPI tag in multiboot tags...");
    Multiboot::MultibootInfo multiboot_info(multiboot_info_addr);

    auto* new_acpi_tag = multiboot_info.FindTag<Multiboot::TagNewAcpi>();

    if (new_acpi_tag == nullptr) {
        TRACE_INFO("ACPI2.0 tag not found in multiboot tags, trying ACPI1.0 tag...");
        auto* old_acpi_tag = multiboot_info.FindTag<Multiboot::TagOldAcpi>();

        new_acpi_tag = reinterpret_cast<Multiboot::TagNewAcpi*>(old_acpi_tag);
    }

    return new_acpi_tag;
}
int ACPI::ACPIController::Init(const u64 multiboot_info_addr)
{
    TODO_WHEN_VMEM_WORKS
    TRACE_INFO("ACPI initialization...");

    TRACE_INFO("Finding RSDP...");
    auto* acpi_tag = FindAcpiTag(multiboot_info_addr);
    R_ASSERT_NOT_NULL(
        acpi_tag, "ACPI tag not found in multiboot tags, only platforms with ACPI supported..."
    );

    TRACE_SUCCESS(
        "ACPI tag found at 0x%0*llX, size: %sB", 2 * sizeof(u64), reinterpret_cast<u64>(acpi_tag),
        FormatMetricUint(acpi_tag->size)
    );

    RsdpAddress_ = reinterpret_cast<void*>(acpi_tag->rsdp);
    TRACE_INFO("RSDP address: 0x%0*llX", 2 * sizeof(u64), RsdpAddress_);

    /* Load all tables, bring the event subsystem online, and enter ACPI mode */
    uacpi_status ret = uacpi_initialize(0);
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_initialize error: %s", uacpi_status_to_string(ret));

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
