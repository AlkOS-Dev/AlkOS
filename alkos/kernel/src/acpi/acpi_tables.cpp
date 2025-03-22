#include <acpi/acpi_tables.hpp>

uacpi_table ACPI::GetTable(ACPI::TableType type)
{
    uacpi_table table;

    using TableType = ACPI::TableType;
    switch (type) {
        case TableType::kRSDT:
            uacpi_table_find_by_signature("RSDT", &table);
            break;
        case TableType::kXSDT:
            uacpi_table_find_by_signature("XSDT", &table);
            break;
        case TableType::kFADT:
            uacpi_table_find_by_signature("FACP", &table);
            break;
        case TableType::kMADT:
            uacpi_table_find_by_signature("APIC", &table);
            break;
        case TableType::kDSDT:
            uacpi_table_find_by_signature("DSDT", &table);
            break;
        case TableType::kSSDT:
            uacpi_table_find_by_signature("SSDT", &table);
            break;
        case TableType::kSRAT:
            uacpi_table_find_by_signature("SRAT", &table);
            break;
    }

    return table;
}
