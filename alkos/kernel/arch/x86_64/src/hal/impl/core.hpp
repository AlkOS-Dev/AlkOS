#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_

#include <hal/api/core.hpp>
#include "cpu/msrs.hpp"
#include "cpu/utils.hpp"
#include "drivers/apic/local_apic.hpp"

namespace arch
{

static constexpr u32 kIa32GsBase = 0xC0000101;

struct CoreConfig {
    u16 acpi_id;
};

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

NODISCARD WRAP_CALL u32 GetCurrentCoreId() { return LocalApic::GetCoreId(); }

FAST_CALL void SetCoreLocalData(void *data)
{
    BlockHardwareInterrupts();
    cpu::SetMSR(kIa32GsBase, reinterpret_cast<u64>(data));
    EnableHardwareInterrupts();
}

FAST_CALL void *GetCoreLocalData() { return reinterpret_cast<void *>(cpu::GetMSR(kIa32GsBase)); }

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_
