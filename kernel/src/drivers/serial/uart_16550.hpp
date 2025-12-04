#ifndef KERNEL_SRC_DRIVERS_SERIAL_UART_16550_HPP_
#define KERNEL_SRC_DRIVERS_SERIAL_UART_16550_HPP_

#include <io/register.hpp>
#include <io/stream.hpp>
#include <types.hpp>

namespace Drivers::Serial
{

class Uart16550 final : public IO::IStream
{
    public:
    // Standard COM ports base addresses for reference
    static constexpr size_t kCom1Base = 0x3F8;
    static constexpr size_t kCom2Base = 0x2F8;

    explicit Uart16550(size_t base_addr);

    /**
     * @brief Initializes the UART controller.
     * Sets baud rate, frame format (8N1), and enables FIFO.
     * @param baud_rate Target baud rate (e.g., 38400, 115200).
     */
    void Init(uint32_t baud_rate = 38400);

    // IO::IStream implementation
    IO::IoResult Write(std::span<const byte> buffer) override;
    IO::IoResult Read(std::span<byte> buffer) override;

    private:
    // ------------------------------
    // Registers
    // ------------------------------

    // Read/Write Data (DLAB=0) or Divisor Low (DLAB=1)
    Io::Register data_reg_;

    // Interrupt Enable (DLAB=0) or Divisor High (DLAB=1)
    Io::Register int_enable_reg_;

    // FIFO Control (Write) / ISR (Read)
    Io::Register fifo_ctrl_reg_;

    // Line Control Register
    Io::Register line_ctrl_reg_;

    // Modem Control Register
    Io::Register modem_ctrl_reg_;

    // Line Status Register
    Io::Register line_status_reg_;

    // ------------------------------
    // Helpers
    // ------------------------------
    NODISCARD bool IsTransmitEmpty() const;
    NODISCARD bool IsDataReady() const;
    void SetBaudRate(uint32_t baud_rate);
};

}  // namespace Drivers::Serial

#endif  // KERNEL_SRC_DRIVERS_SERIAL_UART_16550_HPP_
