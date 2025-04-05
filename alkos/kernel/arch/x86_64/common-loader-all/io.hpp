#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_IO_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_IO_HPP_

#include <defines.hpp>
#include "extensions/types.hpp"

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

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_IO_HPP_
