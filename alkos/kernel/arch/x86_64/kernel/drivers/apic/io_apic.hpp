#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_

#include <defines.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include "acpi/acpi.hpp"
#include "memory_io.hpp"

/**
 * @file io_apic.hpp
 * @brief IO APIC (I/O Advanced Programmable Interrupt Controller) driver
 *
 * The IO APIC is a memory-mapped controller that manages external hardware interrupts
 * and routes them to appropriate Local APICs. Each IO APIC controls a range of
 * Global System Interrupts (GSIs) and can redirect them to any CPU in the system.
 *
 * Access pattern: Write register index to IOREGSEL (offset 0x00), then read/write
 * data at IOWIN (offset 0x10). Both registers are 32-bit.
 *
 * Reference: Intel 64 and IA-32 Architectures Software Developer's Manual
 * - Volume 3A: System Programming Guide, Part 1
 * - Chapter 10, Section 10.11: I/O APIC
 */

// ------------------------------
// Register Definitions
// ------------------------------

/// Register selector offset (write register index here)
static constexpr u64 kIoRegSelOffset = 0x0;

/// Data window offset (read/write register data here)
static constexpr u64 kIoRegWin = 0x10;

/// ID Register (0x00): [24:27] APIC ID, others reserved
static constexpr u32 kIoApicIdReg = 0x00;

/// Version Register (0x01): [0:7] Version, [16:23] Max Redirection Entry (count-1)
static constexpr u32 kIoApicVerReg = 0x01;

/// Arbitration ID Register (0x02): [24:27] Arbitration ID, others reserved
static constexpr u32 kIoApicArbReg = 0x02;

/**
 * @brief Calculates redirection table register index
 *
 * Each entry in the redirection table occupies two 32-bit registers (low and high)
 *
 * @param reg_idx Zero-based index of the redirection table entry
 * @return Register offset for the lower 32 bits of the entry
 */
static constexpr u32 IoApicTableReg(const u32 reg_idx) { return 0x10 + 2 * reg_idx; }

// ------------------------------
// Driver Class
// ------------------------------

/**
 * @brief IO APIC controller class
 *
 * Manages access to a single IO APIC device in the system,
 * handling interrupt routing configuration and GSI management.
 */
class IoApic final
{
    public:
    /**
     * @brief IO APIC redirection table entry (lower 32 bits)
     *
     * Controls how an interrupt is routed to processor(s)
     */
    struct LowerTableRegister {
        enum class TriggerMode : u8 {
            kEdge  = 0,  ///< Edge-triggered interrupt
            kLevel = 1,  ///< Level-triggered interrupt
        };

        enum class DestinationMode : u8 {
            kPhysical = 0,  ///< Physical APIC ID targeting
            kLogical  = 1,  ///< Logical APIC ID targeting
        };

        enum class Mask : u8 {
            kEnabled = 0,  ///< Interrupt enabled
            kMasked  = 1,  ///< Interrupt masked (disabled)
        };

        enum class PinPolarity : u8 {
            kActiveHigh = 0,  ///< Interrupt triggered on high signal
            kActiveLow  = 1,  ///< Interrupt triggered on low signal
        };

        enum class DeliveryStatus : u8 {
            kReadyToSend      = 0,  ///< Ready to accept new interrupt
            kSendNotProcessed = 1,  ///< Delivery in progress
        };

        enum class DeliveryMode : u8 {
            kFixed         = 0b000,  ///< Deliver to specified vector
            kLowerPriority = 0b001,  ///< Deliver to lowest priority processor
            kSMI           = 0b010,  ///< System Management Interrupt
            kNMI           = 0b100,  ///< Non-Maskable Interrupt
            kINIT          = 0b101,  ///< INIT signal
            kExtINT        = 0b111,  ///< External interrupt (like 8259 PIC)
        };

        u32 vector : 8;                        ///< Interrupt vector (0-255)
        DeliveryMode delivery_mode : 3;        ///< How the interrupt should be delivered
        DestinationMode destination_mode : 1;  ///< Physical or logical destination
        DeliveryStatus delivery_status : 1;    ///< 1 if delivery in progress
        PinPolarity pin_polarity : 1;          ///< Signal polarity
        u32 remote_IRR : 1;                    ///< Remote IRR (for level-triggered)
        TriggerMode trigger_mode : 1;          ///< Edge or level triggered
        Mask mask : 1;                         ///< Enable/disable interrupt
        u32 reserved : 15;                     ///< Reserved bits
    };
    static_assert(sizeof(LowerTableRegister) == 4);

    /**
     * @brief IO APIC redirection table entry (upper 32 bits)
     *
     * Contains destination processor information
     */
    struct HigherTableRegister {
        u32 reserved : 24;    ///< Reserved bits
        u32 destination : 8;  ///< Destination APIC ID
    };
    static_assert(sizeof(HigherTableRegister) == 4);

    // ------------------------------
    // Class Creation
    // ------------------------------

    /**
     * @brief Constructor for IO APIC driver
     *
     * @param id IO APIC identifier
     * @param address Physical address of the IO APIC registers
     * @param gsi_base First GSI number managed by this IO APIC
     */
    IoApic(u8 id, u32 address, u32 gsi_base);

    ~IoApic() = default;

    // ------------------------------
    // Register Access Methods
    // ------------------------------

    FORCE_INLINE_F void WriteRegister(const u8 offset, const u32 value) const
    {
        /* Select register we want to operate on by writing to SELECT register */
        WriteMemoryIo<u32>(reinterpret_cast<byte *>(virtual_address_), kIoRegSelOffset, offset);

        /* Write actual value to IO register */
        WriteMemoryIo<u32>(reinterpret_cast<byte *>(virtual_address_), kIoRegWin, value);
    }

    NODISCARD FORCE_INLINE_F u32 ReadRegister(const u8 offset) const
    {
        /* Select register we want to operate on by writing to SELECT register */
        WriteMemoryIo<u32>(reinterpret_cast<byte *>(virtual_address_), kIoRegSelOffset, offset);

        /* Read actual value from IO register */
        return ReadMemoryIo<u32>(reinterpret_cast<byte *>(virtual_address_), kIoRegWin);
    }

    NODISCARD FORCE_INLINE_F LowerTableRegister ReadLowerTableRegister(const u32 reg_idx) const
    {
        return CastRegister<LowerTableRegister>(ReadRegister(IoApicTableReg(reg_idx)));
    }

    FORCE_INLINE_F void WriteLowerTableRegister(
        const u32 reg_idx, const LowerTableRegister &reg_low
    ) const
    {
        WriteRegister(IoApicTableReg(reg_idx), ToRawRegister(reg_low));
    }

    // ------------------------------
    // Status and Configuration Methods
    // ------------------------------

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
    // Class Fields
    // ------------------------------

    private:
    u64 virtual_address_{};   ///< Virtual memory address for MMIO access
    u32 physical_address_{};  ///< Physical memory address of the IO APIC
    u32 gsi_base_{};          ///< First GSI number managed by this IO APIC
    u8 id_{};                 ///< IO APIC ID
    u8 version_{};            ///< IO APIC hardware version
    u8 num_entries_{};        ///< Number of redirection table entries
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_IO_APIC_HPP_
