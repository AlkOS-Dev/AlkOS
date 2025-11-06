#ifndef KERNEL_SRC_ACPI_ACPI_CPU_HPP_
#define KERNEL_SRC_ACPI_ACPI_CPU_HPP_

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
 * @struct Processor
 * @brief Represents a processor in the ACPI namespace.
 */
struct PACK Processor {
    u32 id;        // ACPI processor ID
    bool present;  // Is this processor present?

    u32 apicId;                // Local APIC ID
    u8 maxCState;              // Maximum supported C-state
    u8 maxPState;              // Maximum supported P-state
    bool throttlingSupported;  // Is throttling supported?
};

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // KERNEL_SRC_ACPI_ACPI_CPU_HPP_
