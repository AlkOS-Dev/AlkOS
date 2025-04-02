#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_

template <typename T>
concept SRATEntry = OneOf<
    T, acpi_srat_processor_affinity, acpi_srat_memory_affinity, acpi_srat_x2apic_affinity,
    acpi_srat_gicc_affinity, acpi_srat_gic_its_affinity, acpi_srat_generic_affinity>;

template <typename T>
struct SRATEntryTypeID;

template <>
struct SRATEntryTypeID<acpi_srat_processor_affinity> {
    static constexpr uint8_t value = 0;
};
template <>
struct SRATEntryTypeID<acpi_srat_memory_affinity> {
    static constexpr uint8_t value = 1;
};
template <>
struct SRATEntryTypeID<acpi_srat_x2apic_affinity> {
    static constexpr uint8_t value = 2;
};
template <>
struct SRATEntryTypeID<acpi_srat_gicc_affinity> {
    static constexpr uint8_t value = 3;
};
template <>
struct SRATEntryTypeID<acpi_srat_gic_its_affinity> {
    static constexpr uint8_t value = 4;
};
template <>
struct SRATEntryTypeID<acpi_srat_generic_affinity> {
    static constexpr uint8_t value = 5;
};

template <>
template <typename T>
T* Table<acpi_srat>::GetTableEntry()
{
    static_assert(SRATEntry<T>, "Invalid SRAT entry type");

    constexpr uint8_t target_type = SRATEntryTypeID<T>::value;

    acpi_srat* srat = GetNative();
    if (!srat) {
        return nullptr;
    }

    u8* entry_ptr = reinterpret_cast<u8*>(srat) + sizeof(acpi_srat);
    u8* end_ptr   = reinterpret_cast<u8*>(srat) + srat->hdr.length;

    while (entry_ptr < end_ptr) {
        auto* entry_hdr = reinterpret_cast<acpi_entry_hdr*>(entry_ptr);

        if (entry_hdr->type == target_type) {
            return reinterpret_cast<T*>(entry_ptr);
        }

        entry_ptr += entry_hdr->length;
    }

    return nullptr;
}

#endif  //  ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_
