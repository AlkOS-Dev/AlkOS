#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_

#include <uacpi/uacpi.h>
#include <extensions/defines.hpp>
#include <todo.hpp>

#include "acpi_battery.hpp"
#include "acpi_cpu.hpp"
#include "acpi_device.hpp"
#include "acpi_interrupt.hpp"
#include "acpi_power.hpp"
#include "acpi_tables.hpp"
#include "acpi_thermal.hpp"
#include "sync/mutex.hpp"
#include "sync/spinlock.hpp"

namespace ACPI
{
//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

class ACPIController final
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
    int Init();

    /**
     * @brief Deinitialize the ACPI subsystem.
     */
    WRAP_CALL void Deinit() { uacpi_state_reset(); }

    // ------------------------------
    // Getters
    // ------------------------------

    FORCE_INLINE_F void *GetRsdpAddress() const { return RsdpAddress_; }

    FORCE_INLINE_F Mutex &GetAcpiMutex() { return AcpiMutex_; }

    FORCE_INLINE_F Spinlock &GetAcpiSpinlock() { return AcpiSpinLock_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    TODO_WHEN_VMEM_WORKS
    Mutex AcpiMutex_{};
    Spinlock AcpiSpinLock_{};

    void *RsdpAddress_{};
};

//////////////////////////////
//        Functions         //
//////////////////////////////
}  // namespace ACPI

#include "acpi.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
