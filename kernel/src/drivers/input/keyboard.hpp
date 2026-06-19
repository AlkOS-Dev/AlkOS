// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_DRIVERS_INPUT_KEYBOARD_HPP_
#define KERNEL_SRC_DRIVERS_INPUT_KEYBOARD_HPP_

#include "io/pipe.hpp"
#include "io/stream.hpp"

namespace Drivers::Input
{

/**
 * @brief Abstract Keyboard Driver Base Class.
 *
 * This class decouples the mechanism of receiving keystrokes (Hardware Interrupts)
 * from the mechanism of consuming them (Shell/User-space).
 *
 */
class Keyboard : public IO::IReader
{
    public:
    static constexpr size_t kKeyBufferSize = 128;

    virtual ~Keyboard() = default;

    // ------------------------------
    // IO::IReader Implementation
    // ------------------------------

    /**
     * @brief Reads buffered characters from the keyboard.
     * @param buffer Destination buffer.
     * @return Number of bytes read or Error::Retry if empty.
     */
    IO::IoResult Read(std::span<byte> buffer) override { return pipe_.Read(buffer); }

    // ------------------------------
    // Protected Interface for Drivers
    // ------------------------------

    protected:
    /**
     * @brief Pushes a decoded character into the internal buffer.
     * This method is intended to be called by derived classes (Drivers)
     * typically from an Interrupt Service Routine context.
     *
     * @param c The decoded ASCII character.
     */
    void PushKey(char c)
    {
        byte b = static_cast<byte>(c);
        // If full drop the keystroke
        (void)pipe_.Write(std::span<const byte>(&b, 1));
    }

    private:
    // Lock-free pipe enables IRQ-Main communication
    IO::Pipe<kKeyBufferSize> pipe_;
};

}  // namespace Drivers::Input

#endif  // KERNEL_SRC_DRIVERS_INPUT_KEYBOARD_HPP_
