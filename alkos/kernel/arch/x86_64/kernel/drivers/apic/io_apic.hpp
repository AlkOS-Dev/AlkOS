#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_

#include <defines.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include "acpi/acpi.hpp"
#include "memory_io.hpp"

/**
 * IO APIC Driver
 *
 * Memory-mapped controller that distributes external interrupts to local APICs.
 * Each I/O APIC manages a range of Global System Interrupts (GSIs).
 *
 * Access pattern: write index to IOREGSEL (0x00), then read/write IOWIN (0x10). Both 32bits based.
 *
 * Reference: Intel System Programming Guide, Vol 3A Part 1, Chapter 10
 */

// ------------------------------
// Defines
// ------------------------------

// Offset of select register, should be applied to IO APIC address
static constexpr u64 kIoRegSelOffset = 0x0;

// Offset of write/read register, should be applied to IO APIC address
static constexpr u64 kIoRegWin = 0x10;

// ID Register (0x00): [24:27] APIC ID, others reserved
static constexpr u32 kIoApicIdReg = 0x00;

// Version Register (0x01): [0:7] Version, [16:23] Max Redirection Entry (count-1), others reserved
static constexpr u32 kIoApicVerReg = 0x01;

// Arbitration ID Register (0x02): [24:27] Arbitration ID, others reserved
static constexpr u32 kIoApicArbReg = 0x02;

static constexpr u32 IoApicTableReg(const u32 reg_idx) { return 0x10 + 2 * reg_idx; }

// ------------------------------
// Driver class
// ------------------------------

class IoApic final
{
    public:
    enum class TriggerMode : u8 {
        kEdge  = 0,
        kLevel = 1,
    };

    enum class DestinationMode : u8 {
        kPhysical = 0,
        kLogical  = 1,
    };

    enum class Mask : u8 {
        kEnabled = 0,
        kMasked  = 1,
    };

    enum class PinPolarity : u8 {
        kActiveHigh = 0,
        kActiveLow  = 1,
    };

    enum class DeliveryStatus : u8 {
        kReadyToSend      = 0,
        kSendNotProcessed = 1,
    };

    enum class DeliveryMode : u8 {
        kFixed         = 0b000,
        kLowerPriority = 0b001,
        kSMI           = 0b010,
        kNMI           = 0b100,
        kINIT          = 0b101,
        kExtINT        = 0b111,
    };

    struct LowerTableRegister {
        u32 vector : 8;
        u32 delivery_mode : 3;
        u32 destination_mode : 1;
        u32 delivery_status : 1;
        u32 pin_polarity : 1;
        u32 remote_IRR : 1;
        u32 trigger_mode : 1;
        u32 mask : 1;
        u32 reserved : 15;
    };
    static_assert(sizeof(LowerTableRegister) == 4);

    struct HigherTableRegister {
        u32 reserved : 24;
        u32 destination : 8;
    };
    static_assert(sizeof(HigherTableRegister) == 4);

    // ------------------------------
    // Class defines
    // ------------------------------

    // ------------------------------
    // Class creation
    // ------------------------------

    IoApic(u8 id, u32 address, u32 gsi_base);

    ~IoApic() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void WriteRegister(const u8 offset, const u32 value) const
    {
        /* Select register we want to operate on by writing to SELECT register */
        WriteMemoryIo<u32>(reinterpret_cast<void *>(virtual_address_), kIoRegSelOffset, offset);

        /* Write actual value to IO register */
        WriteMemoryIo<u32>(reinterpret_cast<void *>(virtual_address_), kIoRegWin, value);
    }

    NODISCARD FORCE_INLINE_F u32 ReadRegister(const u8 offset) const
    {
        /* Select register we want to operate on by writing to SELECT register */
        WriteMemoryIo<u32>(reinterpret_cast<void *>(virtual_address_), kIoRegSelOffset, offset);

        /* Read actual value from IO register */
        return ReadMemoryIo<u32>(reinterpret_cast<void *>(virtual_address_), kIoRegWin);
    }

    NODISCARD FORCE_INLINE_F LowerTableRegister ReadLowerTableRegister(const u32 reg_idx) const
    {
        const u32 reg_raw = ReadRegister(IoApicTableReg(reg_idx));
        return *reinterpret_cast<const LowerTableRegister *>(&reg_raw);
    }

    FORCE_INLINE_F void WriteLowerTableRegister(
        const u32 reg_idx, const LowerTableRegister &reg_low
    ) const
    {
        WriteRegister(IoApicTableReg(reg_idx), *reinterpret_cast<const u32 *>(&reg_low));
    }

    NODISCARD u8 GetId() const { return id_; }

    NODISCARD u32 GetAddress() const { return virtual_address_; }

    NODISCARD u32 GetGsiBase() const { return gsi_base_; }

    NODISCARD u8 GetNumEntries() const { return num_entries_; }

    void PrepareDefaultConfig() const;

    NODISCARD bool IsInChargeOfGsi(const u32 gsi) const
    {
        return gsi >= gsi_base_ && gsi < gsi_base_ + num_entries_;
    }

    void ApplyOverrideRule(const acpi_madt_interrupt_source_override *override) const;

    void ApplyNmiRule(const acpi_madt_nmi_source *nmi_source) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    u64 virtual_address_{};
    u32 physical_address_{};
    u32 gsi_base_{};
    u8 id_{};
    u8 version_{};
    u8 num_entries_{};
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
