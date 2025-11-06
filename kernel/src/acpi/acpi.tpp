#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TPP_

namespace ACPI
{

template <typename T>
struct TableSignature;

template <>
struct TableSignature<acpi_rsdt> {
    static constexpr char value[5] = "RSDT";
};
template <>
struct TableSignature<acpi_xsdt> {
    static constexpr char value[5] = "XSDT";
};
template <>
struct TableSignature<acpi_madt> {
    static constexpr char value[5] = "APIC";
};
template <>
struct TableSignature<acpi_fadt> {
    static constexpr char value[5] = "FACP";
};
template <>
struct TableSignature<acpi_dsdt> {
    static constexpr char value[5] = "DSDT";
};
template <>
struct TableSignature<acpi_ssdt> {
    static constexpr char value[5] = "SSDT";
};
template <>
struct TableSignature<acpi_srat> {
    static constexpr char value[5] = "SRAT";
};
template <>
struct TableSignature<acpi_facs> {
    static constexpr char value[5] = "FACS";
};
template <>
struct TableSignature<acpi_mcfg> {
    static constexpr char value[5] = "MCFG";
};
template <>
struct TableSignature<acpi_slit> {
    static constexpr char value[5] = "SLIT";
};
template <>
struct TableSignature<acpi_gtdt> {
    static constexpr char value[5] = "GTDT";
};
template <>
struct TableSignature<acpi_hpet> {
    static constexpr char value[5] = "HPET";
};
template <>
struct TableSignature<acpi_ecdt> {
    static constexpr char value[5] = "ECDT";
};
template <>
struct TableSignature<acpi_rhct> {
    static constexpr char value[5] = "RHCT";
};

}  // namespace ACPI
#endif  //  ALKOS_KERNEL_INCLUDE_ACPI_ACPI_TPP_
