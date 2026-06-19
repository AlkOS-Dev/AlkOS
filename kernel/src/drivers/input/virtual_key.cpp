// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "drivers/input/virtual_key.hpp"

namespace Drivers::Input
{

std::optional<char> VirtualKeyToAscii(VirtualKey vk, KeyModifiers modifiers)
{
    const bool shift = modifiers.shift;
    const bool caps  = modifiers.caps_lock;

    // Check if we need to apply shift for letters
    bool apply_shift_to_letter = shift ^ caps;  // XOR: shift or caps, but not both

    std::optional<char> res;

    switch (vk) {
        // Numbers
        case VK_Key1:
            res.emplace(shift ? '!' : '1');
            break;
        case VK_Key2:
            res.emplace(shift ? '@' : '2');
            break;
        case VK_Key3:
            res.emplace(shift ? '#' : '3');
            break;
        case VK_Key4:
            res.emplace(shift ? '$' : '4');
            break;
        case VK_Key5:
            res.emplace(shift ? '%' : '5');
            break;
        case VK_Key6:
            res.emplace(shift ? '^' : '6');
            break;
        case VK_Key7:
            res.emplace(shift ? '&' : '7');
            break;
        case VK_Key8:
            res.emplace(shift ? '*' : '8');
            break;
        case VK_Key9:
            res.emplace(shift ? '(' : '9');
            break;
        case VK_Key0:
            res.emplace(shift ? ')' : '0');
            break;

        // Letters
        case VK_A:
            res.emplace(apply_shift_to_letter ? 'A' : 'a');
            break;
        case VK_B:
            res.emplace(apply_shift_to_letter ? 'B' : 'b');
            break;
        case VK_C:
            res.emplace(apply_shift_to_letter ? 'C' : 'c');
            break;
        case VK_D:
            res.emplace(apply_shift_to_letter ? 'D' : 'd');
            break;
        case VK_E:
            res.emplace(apply_shift_to_letter ? 'E' : 'e');
            break;
        case VK_F:
            res.emplace(apply_shift_to_letter ? 'F' : 'f');
            break;
        case VK_G:
            res.emplace(apply_shift_to_letter ? 'G' : 'g');
            break;
        case VK_H:
            res.emplace(apply_shift_to_letter ? 'H' : 'h');
            break;
        case VK_I:
            res.emplace(apply_shift_to_letter ? 'I' : 'i');
            break;
        case VK_J:
            res.emplace(apply_shift_to_letter ? 'J' : 'j');
            break;
        case VK_K:
            res.emplace(apply_shift_to_letter ? 'K' : 'k');
            break;
        case VK_L:
            res.emplace(apply_shift_to_letter ? 'L' : 'l');
            break;
        case VK_M:
            res.emplace(apply_shift_to_letter ? 'M' : 'm');
            break;
        case VK_N:
            res.emplace(apply_shift_to_letter ? 'N' : 'n');
            break;
        case VK_O:
            res.emplace(apply_shift_to_letter ? 'O' : 'o');
            break;
        case VK_P:
            res.emplace(apply_shift_to_letter ? 'P' : 'p');
            break;
        case VK_Q:
            res.emplace(apply_shift_to_letter ? 'Q' : 'q');
            break;
        case VK_R:
            res.emplace(apply_shift_to_letter ? 'R' : 'r');
            break;
        case VK_S:
            res.emplace(apply_shift_to_letter ? 'S' : 's');
            break;
        case VK_T:
            res.emplace(apply_shift_to_letter ? 'T' : 't');
            break;
        case VK_U:
            res.emplace(apply_shift_to_letter ? 'U' : 'u');
            break;
        case VK_V:
            res.emplace(apply_shift_to_letter ? 'V' : 'v');
            break;
        case VK_W:
            res.emplace(apply_shift_to_letter ? 'W' : 'w');
            break;
        case VK_X:
            res.emplace(apply_shift_to_letter ? 'X' : 'x');
            break;
        case VK_Y:
            res.emplace(apply_shift_to_letter ? 'Y' : 'y');
            break;
        case VK_Z:
            res.emplace(apply_shift_to_letter ? 'Z' : 'z');
            break;

        // Symbols
        case VK_Minus:
            res.emplace(shift ? '_' : '-');
            break;
        case VK_Equal:
            res.emplace(shift ? '+' : '=');
            break;
        case VK_LeftBracket:
            res.emplace(shift ? '{' : '[');
            break;
        case VK_RightBracket:
            res.emplace(shift ? '}' : ']');
            break;
        case VK_Semicolon:
            res.emplace(shift ? ':' : ';');
            break;
        case VK_Apostrophe:
            res.emplace(shift ? '"' : '\'');
            break;
        case VK_Grave:
            res.emplace(shift ? '~' : '`');
            break;
        case VK_Backslash:
            res.emplace(shift ? '|' : '\\');
            break;
        case VK_Comma:
            res.emplace(shift ? '<' : ',');
            break;
        case VK_Period:
            res.emplace(shift ? '>' : '.');
            break;
        case VK_Slash:
            res.emplace(shift ? '?' : '/');
            break;

        // Whitespace
        case VK_Space:
            res.emplace(' ');
            break;
        case VK_Tab:
            res.emplace('\t');
            break;
        case VK_Enter:
            res.emplace('\n');
            break;
        case VK_Backspace:
            res.emplace('\b');
            break;

        // Escape
        case VK_Escape:
            res.emplace(27);
            break;  // ASCII ESC

        // Numpad (when numlock is on, return digits)
        case VK_NumPad0:
            if (modifiers.num_lock)
                res.emplace('0');
            break;
        case VK_NumPad1:
            if (modifiers.num_lock)
                res.emplace('1');
            break;
        case VK_NumPad2:
            if (modifiers.num_lock)
                res.emplace('2');
            break;
        case VK_NumPad3:
            if (modifiers.num_lock)
                res.emplace('3');
            break;
        case VK_NumPad4:
            if (modifiers.num_lock)
                res.emplace('4');
            break;
        case VK_NumPad5:
            if (modifiers.num_lock)
                res.emplace('5');
            break;
        case VK_NumPad6:
            if (modifiers.num_lock)
                res.emplace('6');
            break;
        case VK_NumPad7:
            if (modifiers.num_lock)
                res.emplace('7');
            break;
        case VK_NumPad8:
            if (modifiers.num_lock)
                res.emplace('8');
            break;
        case VK_NumPad9:
            if (modifiers.num_lock)
                res.emplace('9');
            break;
        case VK_NumPadMultiply:
            res.emplace('*');
            break;
        case VK_NumPadAdd:
            res.emplace('+');
            break;
        case VK_NumPadSubtract:
            res.emplace('-');
            break;
        case VK_NumPadDecimal:
            res.emplace('.');
            break;
        case VK_NumPadDivide:
            res.emplace('/');
            break;
        case VK_NumPadEnter:
            res.emplace('\n');
            break;

        // All other keys don't have ASCII representation
        default:
            break;
    }

    return res;
}

}  // namespace Drivers::Input
