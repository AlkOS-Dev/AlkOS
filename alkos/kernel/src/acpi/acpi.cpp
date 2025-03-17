#include <acpi/acpi.hpp>

/* Internal includes */
#include <extensions/debug.hpp>

/* External includes */
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

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
