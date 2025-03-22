#ifndef ALKOS_KERNEL_INCLUDE_ACPI_CPU_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_CPU_HPP_

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
 * @struct Processor
 * @brief Represents a processor in the ACPI namespace.
 */
STRUCT PACK Processor
{
    u32 id;        // ACPI processor ID
    bool present;  // Is this processor present?

    u32 apicId;                // Local APIC ID
    u8 maxCState;              // Maximum supported C-state
    u8 maxPState;              // Maximum supported P-state
    bool throttlingSupported;  // Is throttling supported?
}
Processor;

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_CPU_HPP_
