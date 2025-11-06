#ifndef KERNEL_ARCH_X86_64_SRC_INCLUDE_IO_HPP_
#define KERNEL_ARCH_X86_64_SRC_INCLUDE_IO_HPP_

#include <assert.h>
#include <defines.hpp>
#include <types.hpp>

FAST_CALL byte inb(const u16 port)
{
    byte v;
    __asm__ volatile("inb %w1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

FAST_CALL u16 inw(const u16 port)
{
    u16 v;
    __asm__ volatile("inw %w1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

FAST_CALL u32 inl(const u16 port)
{
    u32 v;
    __asm__ volatile("inl %w1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

FAST_CALL void outb(const u16 port, const u8 v)
{
    __asm__ volatile("outb %b0, %w1" : : "a"(v), "Nd"(port));
}

FAST_CALL void outw(const u16 port, const u16 v)
{
    __asm__ volatile("outw %w0, %w1" : : "a"(v), "Nd"(port));
}

FAST_CALL void outl(const u16 port, const u32 v)
{
    __asm__ volatile("outl %0, %w1" : : "a"(v), "Nd"(port));
}

/**
 * @brief Hardware delay using I/O port
 *
 * Performs a write to unused port 0x80, typically causing a 1-4μs delay.
 * Primarily used for PIC remapping on old hardware or when precise timing
 * is not required.
 */
FAST_CALL void IoWait() { outb(0x80, 0); }

// ------------------------------
// Cxx bindings
// ------------------------------

#ifdef __cplusplus

#include <concepts_ext.hpp>

namespace io
{
template <concepts_ext::IoT T>
WRAP_CALL T in(const u16 port)
{
    if constexpr (std::is_same_v<T, u8>) {
        return inb(port);
    }

    if constexpr (std::is_same_v<T, u16>) {
        return inw(port);
    }

    if constexpr (std::is_same_v<T, u32>) {
        return inl(port);
    }

    R_FAIL_ALWAYS("Unexpected execution path");
}

template <concepts_ext::IoT T>
WRAP_CALL void out(const u16 port, const T v)
{
    if constexpr (std::is_same_v<T, u8>) {
        outb(port, v);
    } else if constexpr (std::is_same_v<T, u16>) {
        outw(port, v);
    } else if constexpr (std::is_same_v<T, u32>) {
        outl(port, v);
    } else {
        R_FAIL_ALWAYS("Unexpected execution path");
    }
}
}  // namespace io

#endif  // __cplusplus

#endif  // KERNEL_ARCH_X86_64_SRC_INCLUDE_IO_HPP_
