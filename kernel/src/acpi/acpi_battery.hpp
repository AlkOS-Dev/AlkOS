#ifndef KERNEL_SRC_ACPI_ACPI_BATTERY_HPP_
#define KERNEL_SRC_ACPI_ACPI_BATTERY_HPP_

#include <uacpi/tables.h>
#include <defines.hpp>
#include <types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

enum class ChargeState {
    kDischarging      = 0,
    kCharging         = 1,
    kCritical         = 2,
    kChargingCritical = 3,
    kNotCharging      = 4
};

//////////////////////////////
//         Structs          //
//////////////////////////////

/**
 * @struct Battery
 * @brief Represents a battery in the ACPI namespace.
 */
struct PACK Battery {
    u32 id;            // Battery ID
    const char *name;  // Battery name
    bool present;      // Is battery present?

    // Battery information
    const char *model;         // Model number
    const char *serialNumber;  // Serial number
    const char *type;          // Chemistry type
    const char *manufacturer;  // Manufacturer name

    // Battery capacity
    u32 designCapacity;      // Design capacity in mWh
    u32 fullChargeCapacity;  // Full charge capacity in mWh
    u32 currentCapacity;     // Current capacity in mWh

    // Battery state
    u32 currentVoltage;       // Current voltage in mV
    i32 currentRate;          // Current charge/discharge rate in mW
    ChargeState chargeState;  // Charging state

    // Battery health
    u8 healthPercentage;  // Battery health as percentage of original capacity
    u32 cycleCount;       // Charge cycle count

    // Battery capacity warning levels
    u32 warningCapacity;  // Warning level in mWh
    u32 lowCapacity;      // Low level in mWh
};

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // KERNEL_SRC_ACPI_ACPI_BATTERY_HPP_
