#include "drivers/input/ps2_keyboard.hpp"
#include "trace_framework.hpp"

namespace Drivers::Input
{

// ----------------------------------------------------------------------------
// Scancode Maps (Set 1)
// ----------------------------------------------------------------------------

static constexpr char kScancodeMap[128] = {
    0,   27,  '1', '2', '3', '4', '5',  '6', '7', '8',  '9', '0',  '-', '=', '\b', '\t', 'q',
    'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n', 0,   'a', 's',  'd',  'f',
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z', 'x',  'c', 'v', 'b',  'n',  'm',
    ',', '.', '/', 0,   '*', 0,   ' ',  0,   0,   0,    0,   0,    0,   0,   0,    0,    0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,   0,    0,   0,   0,    0,    0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,   0,    0,   0,   0,    0,    0,
    // F11, F12 (mapped to 0 here)
};

static constexpr char kScancodeMapShift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',  '_', '+', '\b', '\t', 'Q',
    'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S',  'D',  'F',
    'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X',  'C', 'V', 'B',  'N',  'M',
    '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,    0,   0,   0,    0,    0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,
};

// ----------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------

Ps2Keyboard::Ps2Keyboard(size_t data_addr, size_t status_addr)
    : data_reg_(data_addr), status_reg_(status_addr)
{
}

void Ps2Keyboard::Init()
{
    TRACE_INFO_GENERAL("Initializing PS/2 Keyboard...");

    // Disable Devices (prevent IRQs while we configure)
    status_reg_.Write<u8>(kCmdDisableKeyboard);  // Disable Keyboard
    status_reg_.Write<u8>(kCmdDisableMouse);     // Disable Mouse

    // Flush Output Buffer (Read garbage)
    while ((status_reg_.Read<u8>() & kStatusOutputBufferFull) != 0) {
        (void)data_reg_.Read<u8>();
    }

    // Set Controller Configuration
    status_reg_.Write<u8>(kCmdReadConfig);  // Read Config
    while ((status_reg_.Read<u8>() & kStatusOutputBufferFull) == 0);
    u8 config = data_reg_.Read<u8>();

    config |= kConfigIrq1Enable;        // Enable IRQ1
    config |= kConfigTranslation;       // Enable Translation
    config &= ~kConfigDisableKeyboard;  // Clear Disable Keyboard bit

    status_reg_.Write<u8>(kCmdWriteConfig);  // Write Config
    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);
    data_reg_.Write<u8>(config);

    // Enable Keyboard Interface
    status_reg_.Write<u8>(kCmdEnableKeyboard);

    // Reset Keyboard Device (Send 0xFF)
    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);  // Wait for input empty
    data_reg_.Write<u8>(kCmdResetDevice);

    // Wait for Reset Response (ACK 0xFA + Success 0xAA)
    // We must consume these bytes so they don't stick the IRQ line as High
    int bytes_expected = 2;
    int timeout        = kResetTimeout;
    while (bytes_expected > 0 && timeout > 0) {
        u8 status = status_reg_.Read<u8>();
        if ((status & kStatusOutputBufferFull) != 0) {
            u8 byte = data_reg_.Read<u8>();
            TRACE_INFO_GENERAL("PS/2 Init: Consumed byte 0x%02x", byte);
            bytes_expected--;
        }
        timeout--;
    }

    if (timeout == 0) {
        TRACE_WARN_GENERAL("PS/2 Reset Timed Out - Keyboard might be unresponsive");
    }

    // Enable Scanning (Send 0xF4)
    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);
    data_reg_.Write<u8>(kCmdEnableScanning);

    // Wait for ACK (0xFA)
    timeout = kResetTimeout;
    while (timeout > 0) {
        if ((status_reg_.Read<u8>() & kStatusOutputBufferFull) != 0) {
            u8 byte = data_reg_.Read<u8>();
            if (byte == kResponseAck) {
                break;
            }
        }
        timeout--;
    }

    TRACE_INFO_GENERAL("PS/2 Keyboard Initialized & Drained");
    trace::Flush();
}

void Ps2Keyboard::OnInterrupt()
{
    TRACE_INFO_HARDWARE("PS/2 Interrupt fired");
    const u8 status = status_reg_.Read<u8>();

    // Check Output Buffer Full (Bit 0)
    if ((status & kStatusOutputBufferFull) != 0) {
        const u8 scancode = data_reg_.Read<u8>();

        // Handle Extended byte (0xE0)
        // This indicates the next byte is part of an extended sequence (e.g., arrow keys)
        if (scancode == kScancodeExtendedPrefix) {
            is_e0_prefix_ = true;
            return;
        }

        // Check for Break Code (Key Release). Bit 7 is set.
        if ((scancode & kScancodeBreakMask) != 0) {
            const u8 released_code = scancode & ~kScancodeBreakMask;

            // Handle Shift Release (Left: 0x2A, Right: 0x36)
            if (released_code == kScancodeLeftShift || released_code == kScancodeRightShift) {
                shift_pressed_ = false;
            }

            // Reset prefix on key release as well
            is_e0_prefix_ = false;
            return;
        }

        // Handle Make Code (Key Press)
        if (scancode == kScancodeLeftShift || scancode == kScancodeRightShift) {  // Shifts
            shift_pressed_ = true;
        } else if (scancode == kScancodeCapsLock) {  // Caps Lock
            caps_lock_ = !caps_lock_;
        } else {
            // Translate and push regular characters
            const char c = TranslateScancode(scancode);
            if (c != 0) {
                PushKey(c);
            }
        }

        is_e0_prefix_ = false;
    }
}

char Ps2Keyboard::TranslateScancode(u8 scancode)
{
    if (scancode > 127) {
        return 0;
    }

    char c = kScancodeMap[scancode];
    if (shift_pressed_) {
        c = kScancodeMapShift[scancode];
    }

    if (caps_lock_) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        } else if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }

    return c;
}

}  // namespace Drivers::Input
