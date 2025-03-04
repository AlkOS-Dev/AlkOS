/**
 * Implementation of serial port communication for QEMU.
 * For detailed implementation reference, see: https://wiki.osdev.org/Serial_Ports
 */

/* internal includes */
#include <assert.h>
#include <debug.hpp>
#include <drivers/serial_port_qemu/serial_qemu.hpp>
#include <io.hpp>

// ------------------------------
// Port and Register Definitions
// ------------------------------

static constexpr u16 kCom1Port                       = 0x3f8;
static constexpr u16 kInterruptEnableReg             = kCom1Port + 1;
static constexpr u16 kReadInterruptIdentificationReg = kCom1Port + 2;
static constexpr u16 kWriteFifoControlReg            = kCom1Port + 2;
static constexpr u16 kLineControlReg                 = kCom1Port + 3;
static constexpr u16 kModemControlReg                = kCom1Port + 4;
static constexpr u16 kReadLineStatusReg              = kCom1Port + 5;
static constexpr u16 kReadModemStatusReg             = kCom1Port + 6;
static constexpr u16 kScratchReg                     = kCom1Port + 7;

static constexpr byte kReceivedBit = 1; /* Indicates data has been received */
static constexpr byte kDlabBit =
    0x80; /* Divisor Latch Access Bit - controls access to baud rate divisor */
static constexpr byte kLineEmpty = 0x20; /* Indicates transmit buffer is empty */

// ------------------------------
// Local Helper Functions
// ------------------------------

/**
 * @brief Check if data is available to be read
 * @return true if data is available, false otherwise
 */
FAST_CALL bool SerialReceived() { return (inb(kReadLineStatusReg) & kReceivedBit) != 0; }

/**
 * @brief Check if the transmission line is empty
 * @return true if the line is empty and ready for transmission, false otherwise
 */
FAST_CALL bool IsLineEmpty() { return (inb(kReadLineStatusReg) & kLineEmpty) != 0; }

// ------------------------------
// Global Interface Functions
// ------------------------------

extern "C" {

/**
 * @brief Initialize the QEMU serial port terminal
 *
 * Configures COM1 port with the following settings:
 * - Baud rate: 38400
 * - Data bits: 8
 * - Stop bits: 1
 * - No parity
 * - FIFO enabled
 * Performs a loopback test to verify the configuration.
 */
void QemuTerminalInit()
{
    /**
     * Line Control Configuration:
     * - Data bits [0-1]: Set to 11 (8 bits per transmission)
     * - Stop bit (2): Set to 0 (single stop bit)
     * - Parity bits [3-5]: Set to 000 (no parity)
     * - DLAB (6) and Break Enable (7): Both disabled
     */
    static constexpr byte kLineControlConfFlags = 0x3;

    /**
     * FIFO Control Configuration (0xC7 = 0b1100'0111):
     * - Bits [6-7]: Set to 11 (14-byte FIFO interrupt threshold)
     * - Bit 2: Set to 1 (clear transmit FIFO)
     * - Bit 1: Set to 1 (clear receive FIFO)
     * - Bit 0: Set to 1 (enable FIFO mode)
     */
    static constexpr byte kFifoConfFlags = 0xC7;

    /**
     * Modem Control Configuration (0x0B = 0b0000'1011):
     * - Bit 0 (DTR): Set to 1 (Data Terminal Ready enabled)
     * - Bit 1 (RTS): Set to 1 (Request To Send enabled)
     * - Bit 3: Set to 1 (Enable IRQs)
     * - Bit 4: Set to 0 (Disable loopback)
     * - Bits [2,5-7]: Unused
     */
    static constexpr byte kModemConfFlags = 0x0B;

    /**
     * Loopback Modem Configuration (0x1A = 0b0001'1010):
     * - Bit 0 (DTR): Set to 0 (Data Terminal Ready disabled)
     * - Bit 1 (RTS): Set to 1 (Request To Send enabled)
     * - Bit 3: Set to 1 (Enable IRQs)
     * - Bit 4: Set to 1 (Enable loopback)
     * - Bits [2,5-7]: Unused
     */
    static constexpr byte kLoopbackModemConfFlags = 0x1A;

    /* Test byte for loopback verification */
    static constexpr byte kTestByte = 0xAE;

    /* Temporarily disable interrupts */
    outb(kInterruptEnableReg, 0);

    /* ------------------------------------------------------------ */
    /* Configure baud rate divisor - must match the receiving end */

    /* Set DLAB to access baud rate divisor registers */
    outb(kLineControlReg, kDlabBit);

    /* Set baud rate divisor to 3 (38400 baud)
     * Lower byte at base port, upper byte at base port + 1 */
    outb(kCom1Port + 0, 3); /* Lower byte */
    outb(kCom1Port + 1, 0); /* Upper byte */

    /* ------------------------------------------------------------ */

    /* Apply line control configuration */
    outb(kLineControlReg, kLineControlConfFlags);

    /* Initialize FIFO with configuration */
    outb(kFifoConfFlags, kFifoConfFlags);

    /* -------------------------------------------------------------- */
    /* Verify port configuration using loopback test */

    /* Enable loopback mode for testing */
    outb(kModemControlReg, kLoopbackModemConfFlags);

    /* Send test byte through loopback */
    outb(kCom1Port, kTestByte);

    /* Verify received byte matches sent byte */
    R_ASSERT_EQ(kTestByte, inb(kCom1Port));

    /* -------------------------------------------------------------- */

    /* Switch to normal operation mode with final modem configuration */
    outb(kModemControlReg, kModemConfFlags);

    TRACE_SUCCESS("QemuTerminalInit() returned with success");
}

/**
 * @brief Send a single character through the serial port
 *
 * @param c The character to transmit
 *
 * Blocks until the transmission line is empty before sending.
 */
void QemuTerminalPutChar(const char c)
{
    while (!IsLineEmpty()) {
    }

    outb(kCom1Port, c);
}

/**
 * @brief Send a null-terminated string through the serial port
 *
 * @param s Pointer to the null-terminated string to transmit
 *
 * Transmits each character in the string sequentially,
 * blocking until each character is sent.
 */
void QemuTerminalWriteString(const char *s)
{
    while (*s) {
        QemuTerminalPutChar(*s);
        ++s;
    }
}

/**
 * @brief Receive a single character from the serial port
 *
 * @return The received character
 *
 * Blocks until a character is received from the serial port.
 */
char QemuTerminalGetChar()
{
    /* Wait until data is available to read */
    while (!SerialReceived()) {
    }

    return static_cast<char>(inb(kCom1Port));
}

/**
 * @brief Read a line of text from the serial port
 *
 * @param buffer Destination buffer for the received line
 * @param size Maximum number of characters to read (including null terminator)
 * @return Number of characters read (including null terminator)
 *
 * Reads characters until either a carriage return ('\r', used by qemu) is received
 * or the buffer is full. The resulting string is null-terminated.
 * The returned size includes the null terminator.
 */
size_t QemuTerminalReadLine(char *buffer, const size_t size)
{
    for (size_t cur = 0; cur < size - 1; ++cur) {
        const char c = QemuTerminalGetChar();

        if (c == '\r') {
            buffer[cur] = '\0';
            return cur + 1;
        }

        buffer[cur] = c;
    }

    buffer[size - 1] = '\0';
    return size;
}
}
