#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include <io.hpp>

namespace pci
{

static constexpr u64 kPCIConfigAddressPort = 0xCF8;
static constexpr u64 kPCIConfigDataPort    = 0xCFC;

template <concepts_ext::IoT T>
T read(u32 bus, u32 device, u32 function, u32 offset)
{
    ASSERT_LT(bus, 256u, "Bus number out of range");
    ASSERT_LT(device, 32u, "Device number out of range");
    ASSERT_LT(function, 8u, "Function number out of range");

    // Adapted from https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);

    static constexpr u16 kOffsetMask = 4 - sizeof(T);
    return io::in<T>(kPCIConfigDataPort + (offset & kOffsetMask));
}

template <concepts_ext::IoT T>
void write(u32 bus, u32 device, u32 function, u32 offset, T value)
{
    ASSERT_LT(bus, 256u, "Bus number out of range");
    ASSERT_LT(device, 32u, "Device number out of range");
    ASSERT_LT(function, 8u, "Function number out of range");

    // Adapted from https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);

    static constexpr u16 kOffsetMask = 4 - sizeof(T);
    io::out(kPCIConfigDataPort + (offset & kOffsetMask), value);
}

}  // namespace pci

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_
