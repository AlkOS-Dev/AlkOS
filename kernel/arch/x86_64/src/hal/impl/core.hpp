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
    void *self;
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

template <typename T, size_t kOffset>
NODISCARD FAST_CALL T GetCoreLocalField()
{
    T value;
    static_assert(sizeof(T) <= 8, "Unsupported size for GetCoreLocalField");

    if constexpr (sizeof(T) == 8) {
        __asm__ volatile("movq %%gs:%1, %q0"
                         : "=r"(value)
                         : "m"(*reinterpret_cast<T *>(kOffset))
                         : "memory");
    } else if constexpr (sizeof(T) == 4) {
        __asm__ volatile("movl %%gs:%1, %k0"
                         : "=r"(value)
                         : "m"(*reinterpret_cast<T *>(kOffset))
                         : "memory");
    } else if constexpr (sizeof(T) == 2) {
        __asm__ volatile("movw %%gs:%1, %w0"
                         : "=r"(value)
                         : "m"(*reinterpret_cast<T *>(kOffset))
                         : "memory");
    } else if constexpr (sizeof(T) == 1) {
        __asm__ volatile("movb %%gs:%1, %b0"
                         : "=r"(value)
                         : "m"(*reinterpret_cast<T *>(kOffset))
                         : "memory");
    }
    return value;
}

template <typename T, size_t Offset>
FAST_CALL void SetCoreLocalField(T value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) <= 8);

    if constexpr (sizeof(T) == 8) {
        __asm__ volatile("movq %q0, %%gs:%P1" : : "r"(value), "n"(Offset) : "memory");
    } else if constexpr (sizeof(T) == 4) {
        __asm__ volatile("movl %k0, %%gs:%P1" : : "r"(value), "n"(Offset) : "memory");
    } else if constexpr (sizeof(T) == 2) {
        __asm__ volatile("movw %w0, %%gs:%P1" : : "r"(value), "n"(Offset) : "memory");
    } else if constexpr (sizeof(T) == 1) {
        __asm__ volatile("movb %b0, %%gs:%P1" : : "r"(value), "n"(Offset) : "memory");
    }
}

void InitializeCoreLocal();

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_CORE_HPP_
