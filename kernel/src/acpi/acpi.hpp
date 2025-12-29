#ifndef KERNEL_SRC_ACPI_ACPI_HPP_
#define KERNEL_SRC_ACPI_ACPI_HPP_

#include <uacpi/uacpi.h>
#include <boot_args.hpp>
#include <defines.hpp>

#include "acpi_battery.hpp"
#include "acpi_cpu.hpp"
#include "acpi_device.hpp"
#include "acpi_interrupt.hpp"
#include "acpi_power.hpp"
#include "acpi_tables.hpp"
#include "acpi_thermal.hpp"
#include "hal/acpi_controller.hpp"

#define R_ASSERT_ACPI_SUCCESS(status, ...) \
    R_ASSERT_EQ(UACPI_STATUS_OK, status __VA_OPT__(, ) __VA_ARGS__)

namespace ACPI
{
//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

class ACPIController final : public hal::AcpiController
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    ACPIController() = default;

    ~ACPIController() = default;

    // ------------------------------
    // Class methods
    // ------------------------------

    /**
     * @brief Early initialize the ACPI subsystem.
     * @param args Boot arguments passed from the bootloader.
     */
    void EarlyInit(const BootArguments &args);

    /**
     * @brief Fully initialize the ACPI subsystem.
     */
    void Init();

    /**
     * @brief Deinitialize the ACPI subsystem.
     */
    WRAP_CALL void Deinit() { uacpi_state_reset(); }

    // ------------------------------
    // Getters
    // ------------------------------

    FORCE_INLINE_F Mem::PPtr<void> GetRsdpAddress() const { return RsdpAddress_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    Mem::PPtr<void> RsdpAddress_{};
};

//////////////////////////////
//        Functions         //
//////////////////////////////
}  // namespace ACPI

#include "acpi.tpp"

#endif  // KERNEL_SRC_ACPI_ACPI_HPP_
