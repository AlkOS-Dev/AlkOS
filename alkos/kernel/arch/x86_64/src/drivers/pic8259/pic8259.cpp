
/* internal includes */
#include "drivers/pic8259/pic8259.hpp"
#include <extensions/debug.hpp>
#include <extensions/types.hpp>

/**
 * @file pic8259.cpp
 * @brief Implementation of the 8259 PIC driver initialization and control functions
 */

// ------------------------------
// Crucial defines
// ------------------------------

/**
 * @name PIC Initialization Command Words (ICW)
 * Configuration bits for the 4-stage initialization sequence:
 * 1. ICW1: Basic initialization and configuration flags
 * 2. ICW2: Interrupt vector offsets in IDT
 * 3. ICW3: Master/Slave relationship configuration
 * 4. ICW4: Additional system configuration
 * @{
 */

static constexpr byte kIcw1Icw4 = 0x01;  ///< Indicates ICW4 will be present
static constexpr byte kIcw1Init = 0x10;  ///< Initialization command bit

static constexpr byte kIcw3Irq2Slave            = 0b100;  ///< Master PIC: IRQ2 has a slave
static constexpr byte kIcw3SlavePicCascadeIdent = 0b10;   ///< Slave PIC: Connected to IRQ2

static constexpr byte kIcw4_8086 = 0x01;  ///< Configure for 8086/88 mode
/** @} */

// ------------------------------
// Implementations
// ------------------------------

/**
 * @brief Initialize both PICs with specified interrupt offsets
 *
 * @param pic1_offset The interrupt vector offset for the master PIC
 * @param pic2_offset The interrupt vector offset for the slave PIC
 *
 * Configures both PICs in cascade mode and sets up their respective
 * interrupt vector offsets in the IDT. Each expects exactly 8 free isrs at the offset.
 */
void InitPic8259(const byte pic1_offset, const byte pic2_offset)
{
    // Cache current interrupt masks to restore them after initialization
    const byte pic1Mask = inb(kPic1Data);
    const byte pic2Mask = inb(kPic2Data);

    // ICW1: Start initialization sequence in cascade mode
    // Both PICs need to be initialized with ICW1 and configured to expect ICW4
    outb(kPic1Command, kIcw1Init | kIcw1Icw4);
    IoWait();
    outb(kPic2Command, kIcw1Init | kIcw1Icw4);
    IoWait();

    // ICW2: Set interrupt vector offsets in IDT
    // This determines the base interrupt numbers for each PIC
    outb(kPic1Data, pic1_offset);
    IoWait();
    outb(kPic2Data, pic2_offset);
    IoWait();

    // ICW3: Configure master/slave relationship
    // Tell master PIC that there's a slave PIC at IRQ2 (0b100)
    outb(kPic1Data, kIcw3Irq2Slave);
    IoWait();
    // Tell slave PIC it's connected to IRQ2 of master (0b010)
    outb(kPic2Data, kIcw3SlavePicCascadeIdent);
    IoWait();

    // ICW4: Set x86 mode for both PICs
    // This configures the PICs for modern x86 mode instead of legacy 8080 mode
    outb(kPic1Data, kIcw4_8086);
    IoWait();
    outb(kPic2Data, kIcw4_8086);
    IoWait();

    // Restore the interrupt masks that were saved at the beginning
    outb(kPic1Data, pic1Mask);
    outb(kPic2Data, pic2Mask);

    TRACE_SUCCESS("PIC units correctly initialized...");
}
