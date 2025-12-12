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

    // 1. Disable Devices (prevent IRQs while we configure)
    status_reg_.Write<u8>(0xAD);  // Disable Keyboard
    status_reg_.Write<u8>(0xA7);  // Disable Mouse

    // 2. Flush Output Buffer (Read garbage)
    while ((status_reg_.Read<u8>() & 1) != 0) {
        (void)data_reg_.Read<u8>();
    }

    // 3. Set Controller Configuration
    status_reg_.Write<u8>(0x20);  // Read Config
    while ((status_reg_.Read<u8>() & 1) == 0);
    u8 config = data_reg_.Read<u8>();

    config |= 0x01;     // Enable IRQ1
    config |= 0x40;     // Enable Translation
    config &= ~(0x10);  // Clear Disable Keyboard bit

    status_reg_.Write<u8>(0x60);  // Write Config
    while ((status_reg_.Read<u8>() & 2) != 0);
    data_reg_.Write<u8>(config);

    // 4. Enable Keyboard Interface
    status_reg_.Write<u8>(0xAE);

    // 5. Reset Keyboard Device (Send 0xFF)
    // This ensures the keyboard is in a known state and clears its internal buffer
    while ((status_reg_.Read<u8>() & 2) != 0);  // Wait for input empty
    data_reg_.Write<u8>(0xFF);

    // 6. Wait for Reset Response (ACK 0xFA + Success 0xAA)
    // We must consume these bytes so they don't stick the IRQ line High!
    int bytes_expected = 2;
    int timeout        = 1000000;
    while (bytes_expected > 0 && timeout > 0) {
        u8 status = status_reg_.Read<u8>();
        if (status & 1) {
            u8 byte = data_reg_.Read<u8>();
            TRACE_INFO_GENERAL("PS/2 Init: Consumed byte 0x%02x", byte);
            bytes_expected--;
        }
        timeout--;
    }

    if (timeout == 0) {
        TRACE_WARN_GENERAL("PS/2 Reset Timed Out - Keyboard might be unresponsive");
    }

    // 7. Enable Scanning (Send 0xF4)
    while ((status_reg_.Read<u8>() & 2) != 0);
    data_reg_.Write<u8>(0xF4);

    // 8. Wait for ACK (0xFA)
    timeout = 1000000;
    while (timeout > 0) {
        if (status_reg_.Read<u8>() & 1) {
            u8 byte = data_reg_.Read<u8>();
            if (byte == 0xFA)
                break;
        }
        timeout--;
    }

    TRACE_INFO_GENERAL("PS/2 Keyboard Initialized & Drained");
    trace::DumpAllBuffersOnFailure();
}

void Ps2Keyboard::OnInterrupt()
{
    DEBUG_INFO_GENERAL("PS/2 Interrupt fired!");
    const u8 status = status_reg_.Read<u8>();

    // Check Output Buffer Full (Bit 0)
    if ((status & 0x01) != 0) {
        const u8 scancode = data_reg_.Read<u8>();

        // Handle Extended byte (0xE0)
        // This indicates the next byte is part of an extended sequence (e.g., arrow keys)
        if (scancode == 0xE0) {
            is_e0_prefix_ = true;
            return;
        }

        // Check for Break Code (Key Release). Bit 7 is set.
        if ((scancode & 0x80) != 0) {
            const u8 released_code = scancode & 0x7F;

            // Handle Shift Release (Left: 0x2A, Right: 0x36)
            if (released_code == 0x2A || released_code == 0x36) {
                shift_pressed_ = false;
            }

            // Reset prefix on key release as well
            is_e0_prefix_ = false;
            return;
        }

        // Handle Make Code (Key Press)
        if (scancode == 0x2A || scancode == 0x36) {  // Shifts
            shift_pressed_ = true;
        } else if (scancode == 0x3A) {  // Caps Lock
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
