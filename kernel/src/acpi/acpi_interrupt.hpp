#ifndef KERNEL_SRC_ACPI_ACPI_INTERRUPT_HPP_
#define KERNEL_SRC_ACPI_ACPI_INTERRUPT_HPP_

#include <defines.hpp>
#include <types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

/**
 * @struct InterruptRoute
 * @brief Represents an interrupt routing entry.
 */
struct PACK InterruptRoute {
    u8 bus;
    u8 device;
    u8 pin;     // INTA, INTB, INTC, or INTD (0-3)
    u8 gsi;     // Global System Interrupt
    u16 flags;  // Polarity, trigger mode, etc.
};

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // KERNEL_SRC_ACPI_ACPI_INTERRUPT_HPP_
