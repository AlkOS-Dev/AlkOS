#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_

#include <uacpi/uacpi.h>
#include <extensions/defines.hpp>

#include "hal/acpi_controller.hpp"
#include "acpi_battery.hpp"
#include "acpi_cpu.hpp"
#include "acpi_device.hpp"
#include "acpi_interrupt.hpp"
#include "acpi_power.hpp"
#include "acpi_tables.hpp"
#include "acpi_thermal.hpp"

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

class ACPIController final : public arch::AcpiController
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
     * @brief Initialize the ACPI subsystem.
     * @return Status code
     */
    int Init(u64 multiboot_info_addr);

    /**
     * @brief Deinitialize the ACPI subsystem.
     */
    WRAP_CALL void Deinit() { uacpi_state_reset(); }

    // ------------------------------
    // Getters
    // ------------------------------

    FORCE_INLINE_F void *GetRsdpAddress() const { return RsdpAddress_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    void *RsdpAddress_{};
};

//////////////////////////////
//        Functions         //
//////////////////////////////
}  // namespace ACPI

#include "acpi.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
