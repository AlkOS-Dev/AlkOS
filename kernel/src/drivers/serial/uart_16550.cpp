#include "drivers/serial/uart_16550.hpp"

#include <hal/sync.hpp>
#include <io/error.hpp>

#include <assert.h>
#include <bits_ext.hpp>

namespace Drivers::Serial
{

// ----------------------------------------------------------------------------
// Register Offsets & Bit Definitions
// ----------------------------------------------------------------------------

static constexpr size_t kRegData       = 0;
static constexpr size_t kRegIntEnable  = 1;
static constexpr size_t kRegFifoCtrl   = 2;
static constexpr size_t kRegLineCtrl   = 3;
static constexpr size_t kRegModemCtrl  = 4;
static constexpr size_t kRegLineStatus = 5;

// Line Control Register Bits
static constexpr u8 kLcrDataBits8  = 0x03;  // 8 bits per character
static constexpr u8 kLcrStopBits1  = 0x00;  // 1 stop bit
static constexpr u8 kLcrParityNone = 0x00;  // No parity
static constexpr u8 kLcrDlab       = 0x80;  // Divisor Latch Access Bit

// FIFO Control Register Bits
static constexpr u8 kFcrEnable    = 0x01;
static constexpr u8 kFcrClearRx   = 0x02;
static constexpr u8 kFcrClearTx   = 0x04;
static constexpr u8 kFcrTrigger14 = 0xC0;  // 14-byte trigger level

// Modem Control Register Bits
static constexpr u8 kMcrDtr  = 0x01;  // Data Terminal Ready
static constexpr u8 kMcrRts  = 0x02;  // Request To Send
static constexpr u8 kMcrOut2 = 0x08;  // Aux Output 2 (Used for IRQs)

// Line Status Register Bits
static constexpr u8 kLsrDataReady = 0x01;
static constexpr u8 kLsrTxEmpty   = 0x20;  // Transmit Holding Register Empty

// Base clock for UART (115200 Hz)
static constexpr u32 kUartBaseClock = 115200;

// ----------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------

Uart16550::Uart16550(size_t base_addr)
    : data_reg_(base_addr, kRegData),
      int_enable_reg_(base_addr, kRegIntEnable),
      fifo_ctrl_reg_(base_addr, kRegFifoCtrl),
      line_ctrl_reg_(base_addr, kRegLineCtrl),
      modem_ctrl_reg_(base_addr, kRegModemCtrl),
      line_status_reg_(base_addr, kRegLineStatus)
{
}

void Uart16550::Init(uint32_t baud_rate)
{
    // Disable interrupts initially
    int_enable_reg_.Write<u8>(0x00);

    // Set Baud Rate
    SetBaudRate(baud_rate);

    // Set Frame Format: 8 bits, 1 stop bit, no parity (8N1)
    // We clear DLAB here as well (bit 7 = 0)
    line_ctrl_reg_.Write<u8>(kLcrDataBits8 | kLcrStopBits1 | kLcrParityNone);

    // Enable and Clear FIFOs
    fifo_ctrl_reg_.Write<u8>(kFcrEnable | kFcrClearRx | kFcrClearTx | kFcrTrigger14);

    // Enable RTS/DTR and Out2 (needed for interrupts on some boards, generally good practice)
    modem_ctrl_reg_.Write<u8>(kMcrDtr | kMcrRts | kMcrOut2);
}

void Uart16550::SetBaudRate(uint32_t baud_rate)
{
    R_ASSERT_NOT_ZERO(baud_rate);

    u32 divisor = kUartBaseClock / baud_rate;

    // Enable DLAB to set divisor
    u8 lcr = line_ctrl_reg_.Read<u8>();
    line_ctrl_reg_.Write<u8>(lcr | kLcrDlab);

    // Write divisor (Low byte then High byte)
    data_reg_.Write<u8>(static_cast<u8>(divisor & kBitMask8));
    int_enable_reg_.Write<u8>(static_cast<u8>((divisor >> 8) & kBitMask8));

    // Disable DLAB (restore previous LCR state, ensuring bit 7 is 0)
    line_ctrl_reg_.Write<u8>(lcr & ~kLcrDlab);
}

bool Uart16550::IsTransmitEmpty() const { return (line_status_reg_.Read<u8>() & kLsrTxEmpty) != 0; }

bool Uart16550::IsDataReady() const { return (line_status_reg_.Read<u8>() & kLsrDataReady) != 0; }

IO::IoResult Uart16550::Read(std::span<byte> buffer)
{
    size_t bytes_read = 0;

    for (size_t i = 0; i < buffer.size(); ++i) {
        // If the hardware has no data, we stop immediately.
        if (!IsDataReady()) {
            // If we haven't read anything yet, tell the caller to try again later.
            if (bytes_read == 0) {
                return std::unexpected(IO::Error::Retry);
            }
            // Otherwise, return what we managed to grab so far (short read).
            break;
        }

        buffer[i] = static_cast<byte>(data_reg_.Read<u8>());
        bytes_read++;
    }

    return bytes_read;
}

IO::IoResult Uart16550::Write(std::span<const byte> buffer)
{
    size_t bytes_written = 0;

    for (const byte b : buffer) {
        // If the transmit holding register/FIFO is full, stop.
        if (!IsTransmitEmpty()) {
            if (bytes_written == 0) {
                return std::unexpected(IO::Error::Retry);
            }
            break;
        }

        data_reg_.Write<u8>(static_cast<u8>(b));
        bytes_written++;
    }

    return bytes_written;
}

}  // namespace Drivers::Serial
