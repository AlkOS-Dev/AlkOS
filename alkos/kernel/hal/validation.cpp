#include <hal/acpi_controller.hpp>
#include <hal/core.hpp>
#include <hal/interrupts.hpp>
#include <hal/spinlock.hpp>

/*
 * This file contains static assertions to verify that all architecture-specific
 * HAL implementations correctly derive from their corresponding ABI classes.
 *
 * This check is performed in a dedicated .cpp file to ensure that the full
 * definitions of the architecture-specific classes are available, preventing
 * "incomplete type" compilation errors that would occur if these checks
 * were placed directly in the ABI headers.
 */

static_assert(
    std::is_base_of_v<arch::AcpiABI, arch::AcpiController>,
    "arch::AcpiController must derive from arch::AcpiABI"
);

static_assert(
    std::is_base_of_v<arch::CoreABI, arch::Core>, "arch::Core must derive from arch::CoreABI"
);

static_assert(
    std::is_base_of_v<arch::InterruptsABI, arch::Interrupts>,
    "arch::Interrupts must derive from arch::InterruptsABI"
);

static_assert(
    std::is_base_of_v<arch::SpinlockAbi, arch::Spinlock>,
    "arch::Spinlock must derive from arch::SpinlockAbi"
);
