#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_UTILS_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_UTILS_TPP_

namespace internal
{

template <typename Table, typename Callback>
void ForEachTableEntry(Table* table, Callback callback)
{
    if (!table) {
        return;
    }

    u8* entry_ptr = reinterpret_cast<u8*>(table) + sizeof(Table);
    u8* end_ptr   = reinterpret_cast<u8*>(table) + table->hdr.length;

    while (entry_ptr < end_ptr) {
        auto* entry = reinterpret_cast<acpi_entry_hdr*>(entry_ptr);
        callback(entry);
        entry_ptr += entry->length;
    }
}

};  // namespace internal

#endif  //  ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_UTILS_TPP_
