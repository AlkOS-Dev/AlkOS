#include <acpi/acpi.hpp>

/* Internal includes */
#include <multiboot2/multiboot2.h>
#include <arch_utils.hpp>
#include <definitions/loader64_data.hpp>
#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>
#include <multiboot2/extensions.hpp>

/* External includes */
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

extern loader64::LoaderData* kLoaderData;

static multiboot::tag_new_acpi_t* FindAcpiTag(u32 multiboot_info_addr)
{
    TRACE_INFO("Finding ACPI tag in multiboot tags...");
    auto* new_acpi_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_new_acpi_t>(
        reinterpret_cast<void*>(multiboot_info_addr)
    );

    if (new_acpi_tag == nullptr) {
        TRACE_INFO("ACPI2.0 tag not found in multiboot tags, trying ACPI1.0 tag...");
        auto* old_acpi_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_old_acpi_t>(
            reinterpret_cast<void*>(multiboot_info_addr)
        );

        new_acpi_tag = reinterpret_cast<multiboot::tag_new_acpi_t*>(old_acpi_tag);
    }

    return new_acpi_tag;
}

int ACPI::ACPIController::Init()
{
    TRACE_INFO("ACPI initialization...");

    TRACE_INFO("Finding RSDP...");
    auto* acpi_tag = FindAcpiTag(kLoaderData->multiboot_info_addr);
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
