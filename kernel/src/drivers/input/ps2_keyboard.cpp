// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "drivers/input/ps2_keyboard.hpp"
#include "modules/input.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

namespace Drivers::Input
{

// ----------------------------------------------------------------------------
// Scancode to VirtualKey Mapping (PS/2 Scancode Set 1)
// ----------------------------------------------------------------------------

Ps2Keyboard::Ps2Keyboard(size_t data_addr, size_t status_addr)
    : data_reg_(data_addr), status_reg_(status_addr)
{
}

VirtualKey Ps2Keyboard::ScancodeToVirtualKey(u8 scancode, bool is_extended)
{
    // Handle extended scancodes (prefixed with 0xE0)
    if (is_extended) {
        switch (scancode) {
            case 0x1C:
                return VK_NumPadEnter;
            case 0x1D:
                return VK_RightCtrl;
            case 0x35:
                return VK_NumPadDivide;
            case 0x38:
                return VK_RightAlt;
            case 0x47:
                return VK_Home;
            case 0x48:
                return VK_ArrowUp;
            case 0x49:
                return VK_PageUp;
            case 0x4B:
                return VK_ArrowLeft;
            case 0x4D:
                return VK_ArrowRight;
            case 0x4F:
                return VK_End;
            case 0x50:
                return VK_ArrowDown;
            case 0x51:
                return VK_PageDown;
            case 0x52:
                return VK_Insert;
            case 0x53:
                return VK_Delete;
            default:
                return VK_Unknown;
        }
    }

    // Normal scancodes
    switch (scancode) {
        case 0x01:
            return VK_Escape;
        case 0x02:
            return VK_Key1;
        case 0x03:
            return VK_Key2;
        case 0x04:
            return VK_Key3;
        case 0x05:
            return VK_Key4;
        case 0x06:
            return VK_Key5;
        case 0x07:
            return VK_Key6;
        case 0x08:
            return VK_Key7;
        case 0x09:
            return VK_Key8;
        case 0x0A:
            return VK_Key9;
        case 0x0B:
            return VK_Key0;
        case 0x0C:
            return VK_Minus;
        case 0x0D:
            return VK_Equal;
        case 0x0E:
            return VK_Backspace;
        case 0x0F:
            return VK_Tab;
        case 0x10:
            return VK_Q;
        case 0x11:
            return VK_W;
        case 0x12:
            return VK_E;
        case 0x13:
            return VK_R;
        case 0x14:
            return VK_T;
        case 0x15:
            return VK_Y;
        case 0x16:
            return VK_U;
        case 0x17:
            return VK_I;
        case 0x18:
            return VK_O;
        case 0x19:
            return VK_P;
        case 0x1A:
            return VK_LeftBracket;
        case 0x1B:
            return VK_RightBracket;
        case 0x1C:
            return VK_Enter;
        case 0x1D:
            return VK_LeftCtrl;
        case 0x1E:
            return VK_A;
        case 0x1F:
            return VK_S;
        case 0x20:
            return VK_D;
        case 0x21:
            return VK_F;
        case 0x22:
            return VK_G;
        case 0x23:
            return VK_H;
        case 0x24:
            return VK_J;
        case 0x25:
            return VK_K;
        case 0x26:
            return VK_L;
        case 0x27:
            return VK_Semicolon;
        case 0x28:
            return VK_Apostrophe;
        case 0x29:
            return VK_Grave;
        case 0x2A:
            return VK_LeftShift;
        case 0x2B:
            return VK_Backslash;
        case 0x2C:
            return VK_Z;
        case 0x2D:
            return VK_X;
        case 0x2E:
            return VK_C;
        case 0x2F:
            return VK_V;
        case 0x30:
            return VK_B;
        case 0x31:
            return VK_N;
        case 0x32:
            return VK_M;
        case 0x33:
            return VK_Comma;
        case 0x34:
            return VK_Period;
        case 0x35:
            return VK_Slash;
        case 0x36:
            return VK_RightShift;
        case 0x37:
            return VK_NumPadMultiply;
        case 0x38:
            return VK_LeftAlt;
        case 0x39:
            return VK_Space;
        case 0x3A:
            return VK_CapsLock;
        case 0x3B:
            return VK_F1;
        case 0x3C:
            return VK_F2;
        case 0x3D:
            return VK_F3;
        case 0x3E:
            return VK_F4;
        case 0x3F:
            return VK_F5;
        case 0x40:
            return VK_F6;
        case 0x41:
            return VK_F7;
        case 0x42:
            return VK_F8;
        case 0x43:
            return VK_F9;
        case 0x44:
            return VK_F10;
        case 0x45:
            return VK_NumLock;
        case 0x46:
            return VK_ScrollLock;
        case 0x47:
            return VK_NumPad7;
        case 0x48:
            return VK_NumPad8;
        case 0x49:
            return VK_NumPad9;
        case 0x4A:
            return VK_NumPadSubtract;
        case 0x4B:
            return VK_NumPad4;
        case 0x4C:
            return VK_NumPad5;
        case 0x4D:
            return VK_NumPad6;
        case 0x4E:
            return VK_NumPadAdd;
        case 0x4F:
            return VK_NumPad1;
        case 0x50:
            return VK_NumPad2;
        case 0x51:
            return VK_NumPad3;
        case 0x52:
            return VK_NumPad0;
        case 0x53:
            return VK_NumPadDecimal;
        case 0x57:
            return VK_F11;
        case 0x58:
            return VK_F12;
        default:
            return VK_Unknown;
    }
}

void Ps2Keyboard::UpdateKeyState(u8 scancode, bool is_pressed)
{
    VirtualKey vk = ScancodeToVirtualKey(scancode, is_e0_prefix_);
    if (vk != VK_CapsLock && vk != VK_NumLock && vk != VK_ScrollLock) {
        key_states_[vk] = is_pressed;
    }

    // Update modifier states
    switch (vk) {
        case VK_LeftShift:
        case VK_RightShift:
            modifiers_.shift = is_pressed;
            break;
        case VK_LeftCtrl:
        case VK_RightCtrl:
            modifiers_.ctrl = is_pressed;
            break;
        case VK_LeftAlt:
        case VK_RightAlt:
            modifiers_.alt = is_pressed;
            break;
        case VK_CapsLock:
            if (is_pressed) {  // Toggle on press only
                modifiers_.caps_lock = !modifiers_.caps_lock;
            }
            break;
        case VK_NumLock:
            if (is_pressed) {  // Toggle on press only
                modifiers_.num_lock = !modifiers_.num_lock;
            }
            break;
        case VK_ScrollLock:
            if (is_pressed) {  // Toggle on press only
                modifiers_.scroll_lock = !modifiers_.scroll_lock;
            }
            break;
        default:
            break;
    }
}

void Ps2Keyboard::Init()
{
    TRACE_INFO_GENERAL("Initializing PS/2 Keyboard...");

    // prevent IRQs while we configure
    status_reg_.Write<u8>(kCmdDisableKeyboard);
    status_reg_.Write<u8>(kCmdDisableMouse);

    // Flush Output Buffer
    while ((status_reg_.Read<u8>() & kStatusOutputBufferFull) != 0) {
        (void)data_reg_.Read<u8>();
    }

    // Set Controller Configuration
    status_reg_.Write<u8>(kCmdReadConfig);
    while ((status_reg_.Read<u8>() & kStatusOutputBufferFull) == 0);
    u8 config = data_reg_.Read<u8>();

    config |= kConfigIrq1Enable;
    config |= kConfigTranslation;
    config &= ~kConfigDisableKeyboard;

    status_reg_.Write<u8>(kCmdWriteConfig);
    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);
    data_reg_.Write<u8>(config);

    status_reg_.Write<u8>(kCmdEnableKeyboard);

    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);
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

    while ((status_reg_.Read<u8>() & kStatusInputBufferFull) != 0);
    data_reg_.Write<u8>(kCmdEnableScanning);

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
}

void Ps2Keyboard::OnInterrupt()
{
    TRACE_FREQ_INFO_HARDWARE("PS/2 Interrupt fired");
    const u8 status = status_reg_.Read<u8>();

    if ((status & kStatusOutputBufferFull) != 0) {
        const u8 scancode = data_reg_.Read<u8>();

        // This indicates the next byte is part of an extended sequence (e.g., arrow keys)
        if (scancode == kScancodeExtendedPrefix) {
            is_e0_prefix_ = true;
            return;
        }

        // Check if this is a break code (key release)
        const bool is_break    = (scancode & kScancodeBreakMask) != 0;
        const u8 base_scancode = scancode & ~kScancodeBreakMask;

        // Update key state
        UpdateKeyState(base_scancode, !is_break);

        // Only process key press events for input routing
        if (!is_break) {
            VirtualKey vk = ScancodeToVirtualKey(base_scancode, is_e0_prefix_);

            // Special handling for Tab to switch sessions
            if (vk == VK_Tab) {
                if (VideoModule::IsInited()) {
                    VideoModule::Get().GetWindowManager().SwitchToNextSession();
                }
                is_e0_prefix_ = false;
                return;
            }

            // Route the key press to the input module
            if (InputModule::IsInited()) {
                InputModule::Get().RouteKey(vk, modifiers_);
            }
        }

        // Reset extended prefix state
        is_e0_prefix_ = false;
    }
}

}  // namespace Drivers::Input
