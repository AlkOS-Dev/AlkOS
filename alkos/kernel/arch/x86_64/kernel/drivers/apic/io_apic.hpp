#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_

#include <defines.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

/**
 * IO APIC Driver
 *
 * Memory-mapped controller that distributes external interrupts to local APICs.
 * Each I/O APIC manages a range of Global System Interrupts (GSIs).
 *
 * Access pattern: write index to IOREGSEL (0x00), then read/write IOWIN (0x10). Both 32bits based.
 *
 * Ref: Intel 82093AA I/O APIC Datasheet, Section 3.2
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
    enum class DeliveryMode : u8 {
        kEdge  = 0,
        kLevel = 1,
    };

    enum class DestinationMode : u8 {
        kPhysical = 0,
        kLogical  = 1,
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

    struct HigherTableRegister {
        u32 reserved : 24;
        u32 destination : 8;
    };

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
        *reinterpret_cast<volatile u32 *>(virtual_address_ + kIoRegSelOffset) = offset;

        /* Write actual value to IO register */
        *reinterpret_cast<volatile u32 *>(physical_address_ + kIoRegWin) = value;
    }

    NODISCARD FORCE_INLINE_F u32 ReadRegister(const u8 offset) const
    {
        /* Select register we want to operate on by writing to SELECT register */
        *reinterpret_cast<volatile u32 *>(virtual_address_ + kIoRegSelOffset) = offset;

        /* Read actual value from IO register */
        return *reinterpret_cast<volatile u32 *>(physical_address_ + kIoRegWin);
    }

    NODISCARD u8 GetId() const { return id_; }

    NODISCARD u32 GetAddress() const { return virtual_address_; }

    NODISCARD u32 GetGsiBase() const { return gsi_base_; }

    NODISCARD u8 GetNumEntries() const { return num_entries_; }

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
