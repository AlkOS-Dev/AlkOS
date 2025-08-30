#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_CMOS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_CMOS_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>
#include "include/io.hpp"

static constexpr u64 kCmosAddressPort = 0x70;
static constexpr u64 kCmosDataPort    = 0x71;

void SetCenturyRegisterAddress(u64 address);

FAST_CALL byte ReadCmosRegister(const byte reg)
{
    outb(kCmosAddressPort, reg);
    return inb(kCmosDataPort);
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_CMOS_CMOS_HPP_
