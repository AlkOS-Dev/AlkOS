#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_

#include <hal/api/core.hpp>
#include "cpu/gdt.hpp"
#include "cpu/msrs.hpp"
#include "drivers/apic/local_apic.hpp"

namespace arch
{

// ------------------------------
// defines
// ------------------------------

static constexpr u32 kIa32FsBase       = 0xC0000100;
static constexpr u32 kIa32GsBase       = 0xC0000101;
static constexpr u32 kIa32GsKernelBase = 0xC0000102;

struct CoreConfig {
    u16 acpi_id;
};

// ------------------------------
// arch::Core
// ------------------------------

class Core : public CoreAPI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Core()  = default;
    ~Core() = default;

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void EnableCore();

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
};

// ------------------------------
// arch::CoreController
// ------------------------------

class CoreController : public CoreControllerAPI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    CoreController()  = default;
    ~CoreController() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
};

// ------------------------------
// arch::CoreLocal
// ------------------------------

struct CoreLocal {
    cpu::GDT gdt;
    cpu::Gdtr gdtr;
    cpu::TSS tss;
};

// ------------------------------
// Helpers
// ------------------------------

NODISCARD WRAP_CALL u32 GetCurrentCoreId() { return LocalApic::GetCoreId(); }

FAST_CALL void SetCoreLocalData(void *data)
// note: Caller is responsible for disabling irqs during this function
{
    cpu::SetMSR(kIa32GsBase, reinterpret_cast<u64>(data));
}

FAST_CALL void *GetCoreLocalData() { return reinterpret_cast<void *>(cpu::GetMSR(kIa32GsBase)); }

void InitializeCoreLocal();

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_
