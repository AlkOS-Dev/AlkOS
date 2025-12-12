#ifndef KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_
#define KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_

#include <drivers/input/keyboard.hpp>
#include <io/register.hpp>
#include <types.hpp>

namespace Drivers::Input
{

/**
 * @brief Driver for a Generic PS/2 Keyboard connected via an 8042 Controller.
 */
class Ps2Keyboard final : public Keyboard
{
    public:
    // Standard 8042 Controller Ports
    static constexpr size_t kDefaultDataPort   = 0x60;
    static constexpr size_t kDefaultStatusPort = 0x64;

    /**
     * @brief Construct a new Ps2 Keyboard driver.
     * @param data_addr I/O address of the data register.
     * @param status_addr I/O address of the status/command register.
     */
    explicit Ps2Keyboard(
        size_t data_addr = kDefaultDataPort, size_t status_addr = kDefaultStatusPort
    );

    /**
     * @brief Initializes the driver and drains any pending garbage in the buffer.
     */
    void Init();

    /**
     * @brief Handle the interrupt request.
     * Should be called by the architecture-specific ISR wrapper.
     */
    void OnInterrupt();

    private:
    // I/O Abstractions
    IO::Register data_reg_;
    IO::Register status_reg_;

    // Translation State Machine
    bool is_e0_prefix_{false};  // Processing an extended scancode?
    bool shift_pressed_{false};
    bool caps_lock_{false};

    // Helpers
    NODISCARD char TranslateScancode(u8 scancode);
};

}  // namespace Drivers::Input

#endif  // KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_
