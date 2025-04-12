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

template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) <= 4)
T read(u32 bus, u32 device, u32 function, u32 offset)
{
    ASSERT_LT(bus, 256, "Bus number out of range");
    ASSERT_LT(device, 32, "Device number out of range");
    ASSERT_LT(function, 8, "Function number out of range");

    // Adapted from https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);
    switch (sizeof(T)) {
        case 1:
            return inb(kPCIConfigDataPort + (offset & 3));
        case 2:
            return inw(kPCIConfigDataPort + (offset & 2));
        case 4:
            return inl(kPCIConfigDataPort);
        default:
            return 0;
    }
}

template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) <= 4)
void write(u32 bus, u32 device, u32 function, u32 offset, T value)
{
    ASSERT_LT(bus, 256, "Bus number out of range");
    ASSERT_LT(device, 32, "Device number out of range");
    ASSERT_LT(function, 8, "Function number out of range");

    // Adapted from https://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_#1
    const u32 address =
        AlignDown((bus << 16) | (device << 11) | (function << 8) | offset | kMsb<u32>, 4);
    outl(address, kPCIConfigAddressPort);
    switch (sizeof(T)) {
        case 1:
            outb(kPCIConfigDataPort + (offset & 3), value);
            break;
        case 2:
            outw(kPCIConfigDataPort + (offset & 2), value);
            break;
        case 4:
            outl(kPCIConfigDataPort, value);
            break;
    }
}

}  // namespace pci

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_PCI_HPP_
