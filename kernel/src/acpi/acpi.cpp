#include <acpi/acpi.hpp>

#include <internal/formats.hpp>
#include "trace_framework.hpp"

// TODO
#include <include/multiboot2/multiboot2.h>
#include <include/multiboot2/multiboot_info.hpp>

/* External includes */
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

static void SanitizeRsdp_(const BootArguments &args)
{
    if (args.raw_args.is_acpi1) {
        struct PACK RSDP_t {
            char Signature[8];
            uint8_t Checksum;
            char OEMID[6];
            uint8_t Revision;
            uint32_t RsdtAddress;
        };

        TRACE_FATAL_BOOT("XDD: %llu", *reinterpret_cast<const u64 *>(args.raw_args.rsdp_struct));

        R_ASSERT_NOT_ZERO(reinterpret_cast<const RSDP_t *>(args.raw_args.rsdp_struct)->RsdtAddress);
        return;
    }

    struct PACK XSDP_t {
        char Signature[8];
        uint8_t Checksum;
        char OEMID[6];
        uint8_t Revision;
        uint32_t RsdtAddress;
        uint32_t Length;
        uint64_t XsdtAddress;
        uint8_t ExtendedChecksum;
        uint8_t reserved[3];
    } __attribute__((packed));

    R_ASSERT_NOT_ZERO(reinterpret_cast<const XSDP_t *>(args.raw_args.rsdp_struct)->XsdtAddress);
}

int ACPI::ACPIController::Init(const BootArguments &args)
{
    TODO_WHEN_VMEM_WORKS
    DEBUG_INFO_BOOT("ACPI initialization...");

    SanitizeRsdp_(args);
    for (size_t offset = 0; offset < kMaxRsdpStructSize; ++offset) {
        rsdp_struct_[offset] = args.raw_args.rsdp_struct[offset];
    }

    /* Load all tables, bring the event subsystem online, and enter ACPI mode */
    uacpi_status ret = uacpi_initialize(0);
    R_ASSERT_ACPI_SUCCESS(ret, "uacpi_initialize error: %s", uacpi_status_to_string(ret));

    /* Load the AML namespace */
    // ret = uacpi_namespace_load();
    // if (uacpi_unlikely_error(ret)) {
    //     DEBUG_FATAL_BOOT("uacpi_namespace_load error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }
    //
    // /* Initialize the namespace */
    // ret = uacpi_namespace_initialize();
    // if (uacpi_unlikely_error(ret)) {
    //     DEBUG_FATAL_BOOT("uacpi_namespace_initialize error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }
    //
    // /*
    //  * Tell uACPI that we have marked all GPEs we wanted for wake. This is needed to
    //  * let uACPI enable all unmarked GPEs that have a corresponding AML handler.
    //  */
    // ret = uacpi_finalize_gpe_initialization();
    // if (uacpi_unlikely_error(ret)) {
    //     DEBUG_FATAL_BOOT("uACPI GPE initialization error: %s", uacpi_status_to_string(ret));
    //     return -1;
    // }

    return 0;
}
