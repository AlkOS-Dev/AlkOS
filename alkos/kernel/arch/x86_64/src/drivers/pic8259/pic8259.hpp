#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_DRIVERS_PIC8259_PIC8259_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_DRIVERS_PIC8259_PIC8259_HPP_

/* internal includes */
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include "include/io.hpp"
#include "interrupts/interrupt_types.hpp"

#include <extensions/types.hpp>

/**
 * @file pic8259.hpp
 * @brief Driver interface for the 8259 Programmable Interrupt Controller (PIC)
 *
 * This header provides the interface for initializing and controlling the 8259 PIC,
 * which is responsible for handling hardware interrupts in x86 systems.
 */

/** @name PIC I/O Ports
 * Base I/O port addresses and command/data port offsets for both PICs
 * @{
 */
static constexpr u16 kPic1Addr    = 0x20;           ///< Base address for master PIC
static constexpr u16 kPic2Addr    = 0xA0;           ///< Base address for slave PIC
static constexpr u16 kPic1Command = kPic1Addr;      ///< Command port for master PIC
static constexpr u16 kPic1Data    = kPic1Addr + 1;  ///< Data port for master PIC
static constexpr u16 kPic2Command = kPic2Addr;      ///< Command port for slave PIC
static constexpr u16 kPic2Data    = kPic2Addr + 1;  ///< Data port for slave PIC
/** @} */

// ------------------------------
// Functions
// ------------------------------

void InitPic8259(byte pic1_offset, byte pic2_offset);

/**
 * @brief Send End of Interrupt (EOI) signal to appropriate PIC(s)
 *
 * @param irq The IRQ number that has been serviced (0-15)
 *
 * Signals the end of interrupt processing to the PIC(s). For IRQs 8-15,
 * EOI must be sent to both the slave and master PICs. For IRQs 0-7,
 * EOI is sent only to the master PIC.
 */
FAST_CALL void Pic8259SendEOI(const u8 irq)
{
    /* code for End Of Interrupt */
    static constexpr byte kPicEOI = 0x20;

    if (irq >= 8) {
        outb(kPic2Command, kPicEOI);
    }

    outb(kPic1Command, kPicEOI);
}

/**
 * @brief Set or clear a specific IRQ bit for pic mask
 *
 * @param irq_line The IRQ line to modify (0-15)
 * @param val true to mask (disable) the IRQ, false to unmask (enable)
 *
 * Modifies the mask for a specific IRQ line. Masked interrupts are ignored
 * by the PIC and will not be forwarded to the CPU.
 */
FAST_CALL void Pic8259SetIrqMaskValue(byte irq_line, const bool val)
{
    u16 port;

    if (irq_line < 8) {
        port = kPic1Data;
    } else {
        port = kPic2Data;
        irq_line -= 8;
    }

    byte mask = inb(port);
    SetBitValue(mask, irq_line, val);
    outb(port, mask);
}

/**
 * @brief Disable all interrupts on both PICs
 *
 * Masks all IRQ lines on both the master and slave PICs, effectively
 * disabling all hardware interrupts they control.
 */
FAST_CALL void Pic8259Disable()
{
    /* mask all interrupts possible */
    outb(kPic1Data, 0xff);
    outb(kPic2Data, 0xff);

    TRACE_INFO("Pic8259 disabled...");
}

intr::InterruptDriver &Pic8259InterruptDriver();

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_DRIVERS_PIC8259_PIC8259_HPP_
