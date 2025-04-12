#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_DEVICE_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_DEVICE_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

/**
 * @struct PCIDevice
 * @brief Represents a PCI device.
 */
struct PACK PCIDevice {
    u8 bus;
    u8 device;
    u8 function;
};

/**
 * @struct Device
 * @brief Represents a device in the ACPI namespace.
 */
struct PACK Device {
    const char* name;        // Device name
    const char* hid;         // Hardware ID
    u32 address;             // Device address
    PCIDevice* pci_context;  // PCI context (nullptr if not a PCI device)
    Device* parent;          // Parent device
    Device* child;           // First child device
    Device* next;            // Next sibling device
};

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_ACPI_DEVICE_HPP_
