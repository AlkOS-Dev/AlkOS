#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_

#include "acpi_utils.tpp"

namespace ACPI
{

template <typename T>
concept SRATEntry = concepts_ext::OneOf<
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
template <TableEntryCallback Callback>
void Table<acpi_srat>::ForEachTableEntry(Callback callback)
{
    internal::ForEachTableEntry<acpi_srat>(GetNative(), callback);
}

}  // namespace ACPI

#endif  //  ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_SRAT_TPP_
