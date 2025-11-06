#ifndef KERNEL_SRC_ACPI_ACPI_THERMAL_HPP_
#define KERNEL_SRC_ACPI_ACPI_THERMAL_HPP_

#include <defines.hpp>
#include <types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

enum class CoolingPolicy {
    kActive,   // Use fans and other active cooling
    kPassive,  // Reduce performance to cool
    kCritical  // Emergency shutdown
};

//////////////////////////////
//         Structs          //
//////////////////////////////

/**
 * @struct ThermalZone
 * @brief Represents a thermal zone in the ACPI namespace.
 */
struct PACK ThermalZone {
    u32 id;
    const char *name;

    // Temperature information
    i32 temperature;   // In millidegrees Celsius
    i32 criticalTemp;  // Critical temperature
    i32 hotTemp;       // Hot temperature

    // Cooling information
    bool hasActiveControl;   // Does this zone support active cooling?
    bool hasPassiveControl;  // Does this zone support passive cooling?
};

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // KERNEL_SRC_ACPI_ACPI_THERMAL_HPP_
