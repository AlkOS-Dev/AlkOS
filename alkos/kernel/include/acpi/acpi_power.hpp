#ifndef ALKOS_KERNEL_INCLUDE_ACPI_POWER_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_POWER_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

enum class PowerState {
    kS0 = 0,  // Working
    kS1 = 1,  // Sleeping with processor context maintained
    kS2 = 2,  // Sleeping with processor context lost
    kS3 = 3,  // Sleeping with memory preserved
    kS4 = 4,  // Hibernation
    kS5 = 5   // Soft off
};

enum class DevicePowerState {
    kD0 = 0,  // Fully on
    kD1 = 1,  // Intermediate power savings
    kD2 = 2,  // Intermediate power savings
    kD3 = 3   // Device powered off
};

enum class PowerSource {
    kBattery,
    kAcAdapter,
};

//////////////////////////////
//         Structs          //
//////////////////////////////

//////////////////////////////
//        Functions         //
//////////////////////////////

/**
 * @brief Shutdown the system.
 * @return bool False if shutdown failed.
 *
 * @remark Function will not return if shutdown succeeds.
 */
bool SystemShutdown();

/**
 * @brief Reboot the system.
 * @return bool False if reboot failed.
 *
 * @remark Function will not return if reboot succeeds.
 */
bool SystemReboot();

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_POWER_HPP_
