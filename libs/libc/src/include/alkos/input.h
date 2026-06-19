// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_INPUT_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_INPUT_H_

#include <stdbool.h>
#include "types.h"

typedef enum : u8 {
    // Special Keys
    VK_Unknown = 0,
    VK_Escape  = 2,

    // Function Keys
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,

    // Number Row
    VK_Key0,
    VK_Key1,
    VK_Key2,
    VK_Key3,
    VK_Key4,
    VK_Key5,
    VK_Key6,
    VK_Key7,
    VK_Key8,
    VK_Key9,

    // Letters
    VK_A,
    VK_B,
    VK_C,
    VK_D,
    VK_E,
    VK_F,
    VK_G,
    VK_H,
    VK_I,
    VK_J,
    VK_K,
    VK_L,
    VK_M,
    VK_N,
    VK_O,
    VK_P,
    VK_Q,
    VK_R,
    VK_S,
    VK_T,
    VK_U,
    VK_V,
    VK_W,
    VK_X,
    VK_Y,
    VK_Z,

    // Symbols
    VK_Minus,         // -
    VK_Equal,         // =
    VK_LeftBracket,   // [
    VK_RightBracket,  // ]
    VK_Semicolon,     // ;
    VK_Apostrophe,    // '
    VK_Grave,         // `
    VK_Backslash,     // backslash
    VK_Comma,         // ,
    VK_Period,        // .
    VK_Slash,         // /

    // Whitespace
    VK_Space,
    VK_Tab,
    VK_Enter,
    VK_Backspace,

    // Modifiers
    VK_LeftShift,
    VK_RightShift,
    VK_LeftCtrl,
    VK_RightCtrl,
    VK_LeftAlt,
    VK_RightAlt,
    VK_CapsLock,
    VK_NumLock,
    VK_ScrollLock,

    // Navigation
    VK_Insert,
    VK_Delete,
    VK_Home,
    VK_End,
    VK_PageUp,
    VK_PageDown,
    VK_ArrowUp,
    VK_ArrowDown,
    VK_ArrowLeft,
    VK_ArrowRight,

    // Numpad
    VK_NumPad0,
    VK_NumPad1,
    VK_NumPad2,
    VK_NumPad3,
    VK_NumPad4,
    VK_NumPad5,
    VK_NumPad6,
    VK_NumPad7,
    VK_NumPad8,
    VK_NumPad9,
    VK_NumPadMultiply,
    VK_NumPadAdd,
    VK_NumPadSubtract,
    VK_NumPadDecimal,
    VK_NumPadDivide,
    VK_NumPadEnter,
} VirtualKey;

typedef struct {
    bool shift : 1;
    bool ctrl : 1;
    bool alt : 1;
    bool caps_lock : 1;
    bool num_lock : 1;
    bool scroll_lock : 1;
} KeyModifiers;

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_INPUT_H_
