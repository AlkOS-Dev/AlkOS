#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TABLES_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TABLES_TPP_

template <ACPI::IsTable T>
bool ACPI::Table<T>::IsValid() const
{
    if (table_.ptr == nullptr)
        return false;

    // FACS doesn't have a checksum since it has OSPM writable fields
    if (TableSignature<T>::value == "FACS")
        return true;

    if (!ValidChecksum_())
        return false;

    return true;
}

template <ACPI::IsTable T>
bool ACPI::Table<T>::ValidChecksum_() const
{
    const u8* bytes = static_cast<u8*>(table_.ptr);
    u8 checksum     = 0;
    for (size_t i = 0; i < table_.hdr->length; ++i) checksum += bytes[i];
    return checksum == 0;
}

template <ACPI::IsTable T>
void ACPI::Table<T>::Unref_()
{
    if (table_.ptr) {
        uacpi_table_unref(&table_);
        table_.ptr = nullptr;
    }
}

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TABLES_TPP_
