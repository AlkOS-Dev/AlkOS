#ifndef KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_
#define KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_

#include <types.h>
#include <array.hpp>
#include <drivers/input/keyboard.hpp>
#include <drivers/input/virtual_key.hpp>
#include <io/register.hpp>

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

    // PS/2 Controller Commands
    static constexpr u8 kCmdDisableKeyboard = 0xAD;
    static constexpr u8 kCmdDisableMouse    = 0xA7;
    static constexpr u8 kCmdEnableKeyboard  = 0xAE;
    static constexpr u8 kCmdReadConfig      = 0x20;
    static constexpr u8 kCmdWriteConfig     = 0x60;

    // PS/2 Keyboard Device Commands
    static constexpr u8 kCmdResetDevice    = 0xFF;
    static constexpr u8 kCmdEnableScanning = 0xF4;

    // Status Register Bits
    static constexpr u8 kStatusOutputBufferFull = 1 << 0;
    static constexpr u8 kStatusInputBufferFull  = 1 << 1;

    // Configuration Byte Bits
    static constexpr u8 kConfigIrq1Enable      = 1 << 0;
    static constexpr u8 kConfigDisableKeyboard = 1 << 4;
    static constexpr u8 kConfigTranslation     = 1 << 6;

    // Device Responses
    static constexpr u8 kResponseAck          = 0xFA;
    static constexpr u8 kResponseResetSuccess = 0xAA;

    // Scancodes (Set 1)
    static constexpr u8 kScancodeExtendedPrefix = 0xE0;
    static constexpr u8 kScancodeBreakMask      = 0x80;
    static constexpr u8 kScancodeLeftShift      = 0x2A;
    static constexpr u8 kScancodeRightShift     = 0x36;
    static constexpr u8 kScancodeCapsLock       = 0x3A;
    static constexpr u8 kScanCodeTab            = 0x0F;

    static constexpr int kResetTimeout = 1000000;

    static constexpr size_t kNumVirtualKeys = 256;

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

    /* @brief Get the current state of a key.
     * @param vk Virtual key code to query.
     * @return true if the key is currently pressed, false otherwise.
     */
    bool GetKeyState(VirtualKey vk) const { return key_states_[static_cast<u8>(vk)]; }

    private:
    // I/O Abstractions
    IO::Register data_reg_;
    IO::Register status_reg_;

    // Key State Tracking
    std::array<bool, kNumVirtualKeys> key_states_{};
    KeyModifiers modifiers_;

    // Translation State Machine
    bool is_e0_prefix_{false};  // Processing an extended scancode?

    // Helpers
    NODISCARD VirtualKey ScancodeToVirtualKey(u8 scancode, bool is_extended);
    void UpdateKeyState(u8 scancode, bool is_pressed);
};

}  // namespace Drivers::Input

#endif  // KERNEL_SRC_DRIVERS_INPUT_PS2_KEYBOARD_HPP_
