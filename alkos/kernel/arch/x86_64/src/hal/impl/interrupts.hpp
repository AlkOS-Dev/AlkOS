#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPTS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPTS_HPP_

#include <hal/api/interrupts.hpp>

#include <extensions/data_structures/array_structures.hpp>
#include <extensions/optional.hpp>

#include <todo.hpp>

#include "drivers/apic/io_apic.hpp"
#include "drivers/apic/local_apic.hpp"
#include "drivers/hpet/hpet.hpp"
#include "interrupts/idt.hpp"

namespace arch
{

static constexpr size_t kPageFaultIsrId = 14;

class Interrupts : public InterruptsAPI
{
    TODO_WHEN_VMEM_WORKS
    static constexpr size_t kTemporaryIoApicTableSize = 8;

    public:
    TODO_WHEN_VMEM_WORKS
    using IoApicTable = data_structures::StaticVector<IoApic, kTemporaryIoApicTableSize>;
    static constexpr size_t kMaxSupportedIsr = 48;

    Interrupts()  = default;
    ~Interrupts() = default;

    // ------------------------------
    // ABI
    // ------------------------------

    void Init();

    void InstallIsrHandler(size_t isr_id, IsrHandler handler);

    // ------------------------------
    // Class methods
    // ------------------------------

    /* First stage init is used to allow correct exception handling in whole memory initialization
     * code */
    void FirstStageInit();

    void ApplyIoApicOverride(const acpi_madt_interrupt_source_override *override);

    void ApplyIoApicNmi(const acpi_madt_nmi_source *nmi_source);

    NODISCARD IoApic &GetIoApicHandler(u32 gsi);

    // ------------------------------
    // Getters
    // ------------------------------

    NODISCARD FORCE_INLINE_F IoApicTable &GetIoApicTable() { return io_apic_table_; }

    NODISCARD FORCE_INLINE_F const IoApicTable &GetIoApicTable() const { return io_apic_table_; }

    /* Note: If APIC is initialized all cores will use apic functionality instead of PIC */
    NODISCARD FORCE_INLINE_F LocalApic &GetLocalApic() { return local_apic_; }

    NODISCARD FORCE_INLINE_F const LocalApic &GetLocalApic() const { return local_apic_; }

    NODISCARD FORCE_INLINE_F std::optional<Hpet> &GetHpet() { return hpet_; }

    NODISCARD FORCE_INLINE_F const std::optional<Hpet> &GetHpet() const { return hpet_; }

    // ------------------------------
    // Protected methods
    // ------------------------------

    protected:
    void InitializeDefaultIdt_();

    // ------------------------------
    // Class fields
    // ------------------------------

    Idt idt_{};
    IoApicTable io_apic_table_{};
    LocalApic local_apic_{};
    std::optional<Hpet> hpet_{};
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPTS_HPP_
