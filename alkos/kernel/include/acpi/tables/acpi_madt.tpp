#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_MADT_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_MADT_TPP_

#include "acpi_utils.tpp"

namespace ACPI
{

template <typename T>
concept MADTEntry = concepts_ext::OneOf<
    T, acpi_madt_lapic, acpi_madt_ioapic, acpi_madt_interrupt_source_override, acpi_madt_nmi_source,
    acpi_madt_lapic_nmi, acpi_madt_lapic_address_override, acpi_madt_iosapic, acpi_madt_lsapic,
    acpi_madt_platform_interrupt_source, acpi_madt_x2apic, acpi_madt_x2apic_nmi, acpi_madt_gicc,
    acpi_madt_gicd, acpi_madt_gic_msi_frame, acpi_madt_gicr, acpi_madt_gic_its,
    acpi_madt_multiprocessor_wakeup, acpi_madt_core_pic, acpi_madt_lio_pic, acpi_madt_ht_pic,
    acpi_madt_eio_pic, acpi_madt_msi_pic, acpi_madt_bio_pic, acpi_madt_lpc_pic, acpi_madt_rintc,
    acpi_madt_imsic, acpi_madt_aplic, acpi_madt_plic>;

template <MADTEntry T>
struct MADTEntryTypeID;

template <>
struct MADTEntryTypeID<acpi_madt_lapic> {
    static constexpr uint8_t value = 0;
};
template <>
struct MADTEntryTypeID<acpi_madt_ioapic> {
    static constexpr uint8_t value = 1;
};
template <>
struct MADTEntryTypeID<acpi_madt_interrupt_source_override> {
    static constexpr uint8_t value = 2;
};
template <>
struct MADTEntryTypeID<acpi_madt_nmi_source> {
    static constexpr uint8_t value = 3;
};
template <>
struct MADTEntryTypeID<acpi_madt_lapic_nmi> {
    static constexpr uint8_t value = 4;
};
template <>
struct MADTEntryTypeID<acpi_madt_lapic_address_override> {
    static constexpr uint8_t value = 5;
};
template <>
struct MADTEntryTypeID<acpi_madt_iosapic> {
    static constexpr uint8_t value = 6;
};
template <>
struct MADTEntryTypeID<acpi_madt_lsapic> {
    static constexpr uint8_t value = 7;
};
template <>
struct MADTEntryTypeID<acpi_madt_platform_interrupt_source> {
    static constexpr uint8_t value = 8;
};
template <>
struct MADTEntryTypeID<acpi_madt_x2apic> {
    static constexpr uint8_t value = 9;
};
template <>
struct MADTEntryTypeID<acpi_madt_x2apic_nmi> {
    static constexpr uint8_t value = 10;
};
template <>
struct MADTEntryTypeID<acpi_madt_gicc> {
    static constexpr uint8_t value = 11;
};
template <>
struct MADTEntryTypeID<acpi_madt_gicd> {
    static constexpr uint8_t value = 12;
};
template <>
struct MADTEntryTypeID<acpi_madt_gic_msi_frame> {
    static constexpr uint8_t value = 13;
};
template <>
struct MADTEntryTypeID<acpi_madt_gicr> {
    static constexpr uint8_t value = 14;
};
template <>
struct MADTEntryTypeID<acpi_madt_gic_its> {
    static constexpr uint8_t value = 15;
};
template <>
struct MADTEntryTypeID<acpi_madt_multiprocessor_wakeup> {
    static constexpr uint8_t value = 16;
};
template <>
struct MADTEntryTypeID<acpi_madt_core_pic> {
    static constexpr uint8_t value = 17;
};
template <>
struct MADTEntryTypeID<acpi_madt_lio_pic> {
    static constexpr uint8_t value = 18;
};
template <>
struct MADTEntryTypeID<acpi_madt_ht_pic> {
    static constexpr uint8_t value = 19;
};
template <>
struct MADTEntryTypeID<acpi_madt_eio_pic> {
    static constexpr uint8_t value = 20;
};
template <>
struct MADTEntryTypeID<acpi_madt_msi_pic> {
    static constexpr uint8_t value = 21;
};
template <>
struct MADTEntryTypeID<acpi_madt_bio_pic> {
    static constexpr uint8_t value = 22;
};
template <>
struct MADTEntryTypeID<acpi_madt_lpc_pic> {
    static constexpr uint8_t value = 23;
};
template <>
struct MADTEntryTypeID<acpi_madt_rintc> {
    static constexpr uint8_t value = 24;
};
template <>
struct MADTEntryTypeID<acpi_madt_imsic> {
    static constexpr uint8_t value = 25;
};
template <>
struct MADTEntryTypeID<acpi_madt_aplic> {
    static constexpr uint8_t value = 26;
};
template <>
struct MADTEntryTypeID<acpi_madt_plic> {
    static constexpr uint8_t value = 27;
};

template <>
template <TableEntryCallback Callback>
void Table<acpi_madt>::ForEachTableEntry(Callback callback)
{
    internal::ForEachTableEntry<acpi_madt>(GetNative(), callback);
}

template <MADTEntry T>
FAST_CALL const T *TryToAccessTheTable(const acpi_entry_hdr *entry)
{
    ASSERT_NOT_NULL(entry);

    if (entry->type == MADTEntryTypeID<T>::value) {
        return reinterpret_cast<const T *>(entry);
    }

    return nullptr;
}

}  // namespace ACPI

#endif  //  ALKOS_KERNEL_INCLUDE_ACPI_TABLES_ACPI_MADT_TPP_
